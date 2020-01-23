#ifndef OPENXR_SWAPCHAIN_HPP
#define OPENXR_SWAPCHAIN_HPP

#include "openxrmanager.hpp"
#include "openxrtexture.hpp"

struct XrSwapchainSubImage;

namespace MWVR
{
    class OpenXRSwapchainImpl;

    class OpenXRSwapchain
    {
    public:
        struct Config
        {
            std::vector<int64_t> requestedFormats{};
            int width = -1;
            int height = -1;
            int samples = -1;
        };

    public:
        OpenXRSwapchain(osg::ref_ptr<OpenXRManager> XR, osg::ref_ptr<osg::State> state, Config config);
        ~OpenXRSwapchain();

    public:
        //! Get the next color buffer.
        //! \return The GL texture ID of the now current swapchain image
        osg::ref_ptr<OpenXRTextureBuffer> prepareNextSwapchainImage();
        //! Release current color buffer. Do not forget to call this after rendering to the color buffer.
        void releaseSwapchainImage();
        //! Prepare for render (set FBO)
        void beginFrame(osg::GraphicsContext* gc);
        //! Finalize render
        void endFrame(osg::GraphicsContext* gc);
        //! Get the view surface
        const XrSwapchainSubImage& subImage(void) const;
        //! Width of the view surface
        int width();
        //! Height of the view surface
        int height();
        //! Samples of the view surface
        int samples();
        //! Get the current texture
        OpenXRTextureBuffer* current();
        //! Get the private implementation
        OpenXRSwapchainImpl& impl() { return *mPrivate; }
        //! Get the private implementation
        const OpenXRSwapchainImpl& impl() const { return *mPrivate; }

    private:
        std::unique_ptr<OpenXRSwapchainImpl> mPrivate;
    };
}

#endif
