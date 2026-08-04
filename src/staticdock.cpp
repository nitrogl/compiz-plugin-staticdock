/*
 * Copyright (C) 2026 Roberto Metere
 */

#include <staticdock.h>

COMPIZ_PLUGIN_20090315 (staticdock, StaticDockPluginVTable)

bool
StaticDock::isStatic (CompWindow *w)
{
    bool dock = (w->type () & CompWindowTypeDockMask) != 0;
    
    bool cubeActive = screen->grabExist ("cube") ||
    screen->grabExist ("rotate");
    bool expoActive = screen->grabExist ("expo");
    
    if (cubeActive)
    {
        return optionGetCubeStaticDocks () && dock;
    }
    
    if (expoActive)
    {
        return optionGetExpoStaticDocks () && dock;
    }
    
    // All other effects
    return optionGetGlobalStaticDocks () && dock;
    
}

void
StaticDock::preparePaint (int msSinceLastPaint)
{
    /* Reset per-frame state. */
    mSawTransformedPass    = false;
    mPaintingDocksDirectly = false;
    
    cScreen->preparePaint (msSinceLastPaint);
}

bool
StaticDock::glPaintOutput (const GLScreenPaintAttrib &attrib,
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
StaticDock::glPaintTransformedOutput (const GLScreenPaintAttrib &attrib,
                                            const GLMatrix            &matrix,
                                            const CompRegion          &region,
                                            CompOutput                *output,
                                            unsigned int              mask)
{
    mSawTransformedPass = true;
    
    gScreen->glPaintTransformedOutput (attrib, matrix, region, output, mask);
}

void
StaticDock::paintDocksFlat (CompOutput *output)
{
    GLMatrix flatTransform;
    CompRegion outputRegion (*output);
    
    flatTransform.toScreenSpace (output, -DEFAULT_Z_CAMERA);
    mPaintingDocksDirectly = true;
    
    foreach (CompWindow *w, screen->windows ())
    {
        if (!isStatic (w))
            continue;
        
        if (w->destroyed () || !w->isViewable ())
            continue;
        
        GLWindow *gw = GLWindow::get (w);
        
        if (!gw)
            continue;
        
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
    
    if (sds->mPaintingDocksDirectly)
        return gWindow->glPaint (attrib, matrix, region, mask);
    
    if (sds->mSawTransformedPass && sds->isStatic (window))
        return false;
    
    return gWindow->glPaint (attrib, matrix, region, mask);
}

StaticDock::StaticDock (CompScreen *screen) :
    PluginClassHandler<StaticDock, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    mSawTransformedPass (false),
    mPaintingDocksDirectly (false)
{
    GLScreenInterface::setHandler (gScreen);
    CompositeScreenInterface::setHandler (cScreen);
}

StaticDock::~StaticDock ()
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
