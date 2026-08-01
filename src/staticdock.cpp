/*
 * Copyright (C) 2026 Roberto Metere
 */

#include "staticdock.h"

COMPIZ_PLUGIN_20090315 (staticdock, StaticDockPluginVTable)

void
StaticDockScreen::preparePaint (int msSinceLastPaint)
{
    /* Reset per-frame state. */
    mSawTransformedPass    = false;
    mPaintingDocksDirectly = false;
    
    cScreen->preparePaint (msSinceLastPaint);
}

bool
StaticDockScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
                                 const GLMatrix            &matrix,
                                 const CompRegion          &region,
                                 CompOutput                *output,
                                 unsigned int              mask)
{
    mSawTransformedPass = false;
    
    bool status = gScreen->glPaintOutput (attrib, matrix, region,
                                          output, mask);
    
    if (mSawTransformedPass)
        paintDocksFlat (output);
    
    return status;
}

void
StaticDockScreen::glPaintTransformedOutput (const GLScreenPaintAttrib &attrib,
                                            const GLMatrix            &matrix,
                                            const CompRegion          &region,
                                            CompOutput                *output,
                                            unsigned int              mask)
{
    /* Just a sentinel: record that a transformed pass happened this
     * frame, then let it proceed completely unmodified. We do not try
     * to influence this call in any way - cube owns it. */
    mSawTransformedPass = true;
    
    gScreen->glPaintTransformedOutput (attrib, matrix, region, output, mask);
}

void
StaticDockScreen::paintDocksFlat (CompOutput *output)
{
    GLMatrix flatTransform;
    
    /* Same construction as the plain (non-transformed) branch of
     * GLScreen::glPaintOutput uses for ordinary desktop compositing:
     * no rotation, just the standard camera/screen-space setup. */
    flatTransform.toScreenSpace (output, -DEFAULT_Z_CAMERA);
    
    CompRegion outputRegion (*output); /* CompOutput derives from CompRect */
    
    mPaintingDocksDirectly = true;
    
    foreach (CompWindow *w, screen->windows ())
    {
        if (!isDock (w))
            continue;
        
        if (w->destroyed () || !w->isViewable ())
            continue;
        
        GLWindow *gw = GLWindow::get (w);
        
        if (!gw)
            continue;
        
        /* IMPORTANT: this must be a clean *window*-level mask, built
         * from scratch - never OR'd in with the screen-level mask
         * from glPaintOutput. PAINT_SCREEN_* and PAINT_WINDOW_* bits
         * share the same numeric bit positions (e.g.
         * PAINT_SCREEN_FULL_MASK == PAINT_WINDOW_OCCLUSION_DETECTION_MASK,
         * both bit 1) so passing a screen mask into glPaint gets
         * silently reinterpreted as unrelated window flags - in this
         * case, as "occlusion-detection dry run, don't actually draw",
         * which is exactly why nothing showed up. Match the "clone"
         * plugin's own manual-repaint call, which uses only this one
         * flag and nothing else. */
        gw->glPaint (gw->paintAttrib (), flatTransform, outputRegion,
                     PAINT_WINDOW_ON_TRANSFORMED_SCREEN_MASK);
    }
    
    mPaintingDocksDirectly = false;
}

bool
StaticDockWindow::glPaint (const GLWindowPaintAttrib &attrib,
                           const GLMatrix            &matrix,
                           const CompRegion          &region,
                           unsigned int              mask)
{
    STATICDOCK_SCREEN (screen);
    
    /* Our own direct re-paint call in paintDocksFlat(): always let it
     * through, regardless of what pass we think we're in. Note this
     * only ever runs for dock windows anyway, since paintDocksFlat only
     * calls glPaint on dock windows in the first place - unlike the
     * screen-level mPaintingDocksDirectly bypass in the previous patch,
     * there is no risk of this admitting non-dock windows. */
    if (sds->mPaintingDocksDirectly)
        return gWindow->glPaint (attrib, matrix, region, mask);
    
    /* Inside cube's transformed pass: docks sit this one out, they get
     * painted flat, separately, right after. */
    if (sds->mSawTransformedPass && isDock (window))
        return false;
    
    return gWindow->glPaint (attrib, matrix, region, mask);
}

StaticDockScreen::StaticDockScreen (CompScreen *screen) :
PluginClassHandler<StaticDockScreen, CompScreen> (screen),
cScreen (CompositeScreen::get (screen)),
gScreen (GLScreen::get (screen)),
mSawTransformedPass (false),
mPaintingDocksDirectly (false)
{
    GLScreenInterface::setHandler (gScreen);
    CompositeScreenInterface::setHandler (cScreen);
}

StaticDockScreen::~StaticDockScreen ()
{
}

StaticDockWindow::StaticDockWindow (CompWindow *window) :
PluginClassHandler<StaticDockWindow, CompWindow> (window),
window (window),
gWindow (GLWindow::get (window))
{
    GLWindowInterface::setHandler (gWindow);
}

bool
StaticDockPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION)          ||
        !CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
        return false;
    
    return true;
}
