#include "dialogueextensions.hpp"

/*
    Start of tes3mp addition

    Include additional headers for multiplayer purposes
*/
#include "../mwbase/windowmanager.hpp"
#include "../mwmp/Main.hpp"
#include "../mwmp/LocalPlayer.hpp"
/*
    End of tes3mp addition
*/

#include <components/compiler/extensions.hpp>
#include <components/compiler/opcodes.hpp>
#include <components/debug/debuglog.hpp>
#include <components/interpreter/interpreter.hpp>
#include <components/interpreter/runtime.hpp>
#include <components/interpreter/opcodes.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/dialoguemanager.hpp"
#include "../mwbase/journal.hpp"
#include "../mwbase/world.hpp"
#include "../mwbase/windowmanager.hpp"

#include "../mwworld/class.hpp"
#include "../mwmechanics/npcstats.hpp"

#include "interpretercontext.hpp"
#include "ref.hpp"

namespace MWScript
{
    namespace Dialogue
    {
        template <class R>
        class OpJournal : public Interpreter::Opcode0
        {
            public:

                void execute (Interpreter::Runtime& runtime) override
                {
                    MWWorld::Ptr ptr = R()(runtime, false); // required=false
                    if (ptr.isEmpty())
                        ptr = MWBase::Environment::get().getWorld()->getPlayerPtr();

                    std::string quest = runtime.getStringLiteral (runtime[0].mInteger);
                    runtime.pop();

                    Interpreter::Type_Integer index = runtime[0].mInteger;
                    runtime.pop();

                    // Invoking Journal with a non-existing index is allowed, and triggers no errors. Seriously? :(
                    try
                    {
                        /*
                            Start of tes3mp addition

                            Send an ID_PLAYER_JOURNAL packet every time a new journal entry is added
                            through a script
                        */
                        if (mwmp::Main::get().getLocalPlayer()->isLoggedIn() && !MWBase::Environment::get().getJournal()->hasEntry(quest, index))
                            mwmp::Main::get().getLocalPlayer()->sendJournalEntry(quest, index, ptr);
                        /*
                            End of tes3mp addition
                        */

                        MWBase::Environment::get().getJournal()->addEntry (quest, index, ptr);
                    }
                    catch (...)
                    {
                        if (MWBase::Environment::get().getJournal()->getJournalIndex(quest) < index)
                            MWBase::Environment::get().getJournal()->setJournalIndex(quest, index);
                    }
                }
        };

        class OpSetJournalIndex : public Interpreter::Opcode0
        {
            public:

                void execute (Interpreter::Runtime& runtime) override
                {
                    std::string quest = runtime.getStringLiteral (runtime[0].mInteger);
                    runtime.pop();

                    Interpreter::Type_Integer index = runtime[0].mInteger;
                    runtime.pop();

                    MWBase::Environment::get().getJournal()->setJournalIndex (quest, index);

                    /*
                        Start of tes3mp addition

                        Send an ID_PLAYER_JOURNAL packet every time a journal index is set
                        through a script
                    */
                    if (mwmp::Main::get().getLocalPlayer()->isLoggedIn())
                        mwmp::Main::get().getLocalPlayer()->sendJournalIndex(quest, index);
                    /*
                        End of tes3mp addition
                    */
                }
        };

        class OpGetJournalIndex : public Interpreter::Opcode0
        {
            public:

                void execute (Interpreter::Runtime& runtime) override
                {
                    std::string quest = runtime.getStringLiteral (runtime[0].mInteger);
                    runtime.pop();

                    int index = MWBase::Environment::get().getJournal()->getJournalIndex (quest);

                    runtime.push (index);

                }
        };

        class OpAddTopic : public Interpreter::Opcode0
        {
            public:

                void execute (Interpreter::Runtime& runtime) override
                {
                    std::string topic = runtime.getStringLiteral (runtime[0].mInteger);
                    runtime.pop();

                    /*
                        Start of tes3mp addition

                        Send an ID_PLAYER_TOPIC packet every time a new topic is added
                        through a script
                    */
                    if (mwmp::Main::get().getLocalPlayer()->isLoggedIn() &&
                        MWBase::Environment::get().getDialogueManager()->isNewTopic(Misc::StringUtils::lowerCase(topic)))
                        mwmp::Main::get().getLocalPlayer()->sendTopic(Misc::StringUtils::lowerCase(topic));
                    /*
                        End of tes3mp addition
                    */

                    MWBase::Environment::get().getDialogueManager()->addTopic(topic);
                }
        };

        class OpChoice : public Interpreter::Opcode1
        {
            public:

                void execute (Interpreter::Runtime& runtime, unsigned int arg0) override
                {
                    MWBase::DialogueManager* dialogue = MWBase::Environment::get().getDialogueManager();
                    while(arg0>0)
                    {
                        std::string question = runtime.getStringLiteral (runtime[0].mInteger);
                        runtime.pop();
                        arg0 = arg0 -1;
                        Interpreter::Type_Integer choice = 1;
                        if(arg0>0)
                        {
                            choice = runtime[0].mInteger;
                            runtime.pop();
                            arg0 = arg0 -1;
                        }
                        dialogue->addChoice(question,choice);
                    }
                }
        };

        template<class R>
        class OpForceGreeting : public Interpreter::Opcode0
        {
            public:

                void execute (Interpreter::Runtime& runtime) override
                {
                    MWWorld::Ptr ptr = R()(runtime);

                    if (!ptr.getRefData().isEnabled())
                        return;

                    if (!ptr.getClass().isActor())
                    {
                        const std::string error = "Warning: \"forcegreeting\" command works only for actors.";
                        runtime.getContext().report(error);
                        Log(Debug::Warning) << error;
                        return;
                    }

                    /*
                        Start of tes3mp change (major)

                        Don't start a dialogue if the target is already engaged in one, thus
                        preventing infinite greeting loops
                    */
                    if (!MWBase::Environment::get().getWindowManager()->containsMode(MWGui::GM_Dialogue))
                        MWBase::Environment::get().getWindowManager()->pushGuiMode(MWGui::GM_Dialogue, ptr);
                    /*
                        End of tes3mp change (major)
                    */
                }
        };

        class OpGoodbye : public Interpreter::Opcode0
        {
            public:

                void execute(Interpreter::Runtime& runtime) override
                {
                    MWBase::Environment::get().getDialogueManager()->goodbye();
                }
        };

        template<class R>
        class OpModReputation : public Interpreter::Opcode0
        {
            public:

                void execute (Interpreter::Runtime& runtime) override
                {
                    MWWorld::Ptr ptr = R()(runtime);
                    Interpreter::Type_Integer value = runtime[0].mInteger;
                    runtime.pop();

                    ptr.getClass().getNpcStats (ptr).setReputation (ptr.getClass().getNpcStats (ptr).getReputation () + value);
                }
        };

        template<class R>
        class OpSetReputation : public Interpreter::Opcode0
        {
            public:

                void execute (Interpreter::Runtime& runtime) override
                {
                    MWWorld::Ptr ptr = R()(runtime);
                    Interpreter::Type_Integer value = runtime[0].mInteger;
                    runtime.pop();

                    ptr.getClass().getNpcStats (ptr).setReputation (value);
                }
        };

        template<class R>
        class OpGetReputation : public Interpreter::Opcode0
        {
            public:

                void execute (Interpreter::Runtime& runtime) override
                {
                    MWWorld::Ptr ptr = R()(runtime);

                    runtime.push (ptr.getClass().getNpcStats (ptr).getReputation ());
                }
        };

        template<class R>
        class OpSameFaction : public Interpreter::Opcode0
        {
            public:

                void execute (Interpreter::Runtime& runtime) override
                {
                    MWWorld::Ptr ptr = R()(runtime);

                    MWWorld::Ptr player = MWBase::Environment::get().getWorld ()->getPlayerPtr();

                    runtime.push(player.getClass().getNpcStats (player).isInFaction(ptr.getClass().getPrimaryFaction(ptr)));
                }
        };

        class OpModFactionReaction : public Interpreter::Opcode0
        {
        public:

            void execute (Interpreter::Runtime& runtime) override
            {
                std::string faction1 = runtime.getStringLiteral (runtime[0].mInteger);
                runtime.pop();

                std::string faction2 = runtime.getStringLiteral (runtime[0].mInteger);
                runtime.pop();

                int modReaction = runtime[0].mInteger;
                runtime.pop();

                MWBase::Environment::get().getDialogueManager()->modFactionReaction(faction1, faction2, modReaction);
            }
        };

        class OpGetFactionReaction : public Interpreter::Opcode0
        {
        public:

            void execute (Interpreter::Runtime& runtime) override
            {
                std::string faction1 = runtime.getStringLiteral (runtime[0].mInteger);
                runtime.pop();

                std::string faction2 = runtime.getStringLiteral (runtime[0].mInteger);
                runtime.pop();

                runtime.push(MWBase::Environment::get().getDialogueManager()
                             ->getFactionReaction(faction1, faction2));
            }
        };

        class OpSetFactionReaction : public Interpreter::Opcode0
        {
        public:

            void execute (Interpreter::Runtime& runtime) override
            {
                std::string faction1 = runtime.getStringLiteral (runtime[0].mInteger);
                runtime.pop();

                std::string faction2 = runtime.getStringLiteral (runtime[0].mInteger);
                runtime.pop();

                int newValue = runtime[0].mInteger;
                runtime.pop();

                MWBase::Environment::get().getDialogueManager()->setFactionReaction(faction1, faction2, newValue);
            }
        };

        template <class R>
        class OpClearInfoActor : public Interpreter::Opcode0
        {
        public:
            void execute (Interpreter::Runtime& runtime) override
            {
                MWWorld::Ptr ptr = R()(runtime);

                MWBase::Environment::get().getDialogueManager()->clearInfoActor(ptr);
            }
        };


        void installOpcodes (Interpreter::Interpreter& interpreter)
        {
            interpreter.installSegment5 (Compiler::Dialogue::opcodeJournal, new OpJournal<ImplicitRef>);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeJournalExplicit, new OpJournal<ExplicitRef>);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeSetJournalIndex, new OpSetJournalIndex);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeGetJournalIndex, new OpGetJournalIndex);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeAddTopic, new OpAddTopic);
            interpreter.installSegment3 (Compiler::Dialogue::opcodeChoice,new OpChoice);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeForceGreeting, new OpForceGreeting<ImplicitRef>);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeForceGreetingExplicit, new OpForceGreeting<ExplicitRef>);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeGoodbye, new OpGoodbye);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeGetReputation, new OpGetReputation<ImplicitRef>);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeSetReputation, new OpSetReputation<ImplicitRef>);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeModReputation, new OpModReputation<ImplicitRef>);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeSetReputationExplicit, new OpSetReputation<ExplicitRef>);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeModReputationExplicit, new OpModReputation<ExplicitRef>);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeGetReputationExplicit, new OpGetReputation<ExplicitRef>);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeSameFaction, new OpSameFaction<ImplicitRef>);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeSameFactionExplicit, new OpSameFaction<ExplicitRef>);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeModFactionReaction, new OpModFactionReaction);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeSetFactionReaction, new OpSetFactionReaction);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeGetFactionReaction, new OpGetFactionReaction);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeClearInfoActor, new OpClearInfoActor<ImplicitRef>);
            interpreter.installSegment5 (Compiler::Dialogue::opcodeClearInfoActorExplicit, new OpClearInfoActor<ExplicitRef>);
        }
    }

}
