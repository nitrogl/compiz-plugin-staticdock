/*
 * Copyright (C) 2026 Roberto Metere
 */

#ifndef _STATICDOCK_H
#define _STATICDOCK_H


#include <core/core.h>
#include <core/pluginclasshandler.h>
#include <core/match.h>
#include <composite/composite.h>
#include <opengl/opengl.h>

#include "staticdock_options.h"


enum StaticDockEffect
{
    StaticDockNone,
    StaticDockCube,
    StaticDockExpo
};

class StaticDock :
public PluginClassHandler<StaticDock, CompScreen>,
public StaticdockOptions,
public GLScreenInterface,
public CompositeScreenInterface
{
public:
    StaticDock (CompScreen *screen);
    ~StaticDock ();
    
    CompositeScreen *cScreen;
    GLScreen        *gScreen;
    
    /* True once we've observed a transformed (cube) pass this frame */
    bool mSawTransformedPass;
    
    /* Main idea
     * True only while we are manually re-painting dock windows
     * ourselves after the main pass; lets StaticDockWindow::glPaint
     * tell "cube's pass, please skip this dock" apart from
     * "our own direct call, please paint regardless." */
    bool mPaintingDocksDirectly;
    
    void preparePaint (int msSinceLastPaint);
    
    bool glPaintOutput (const GLScreenPaintAttrib &attrib,
                        const GLMatrix            &matrix,
                        const CompRegion          &region,
                        CompOutput                *output,
                        unsigned int              mask);
    
    void glPaintTransformedOutput (const GLScreenPaintAttrib &attrib,
                                   const GLMatrix            &matrix,
                                   const CompRegion          &region,
                                   CompOutput                *output,
                                   unsigned int              mask);
    
    void paintDocksFlat (CompOutput *output);
    
    bool isStatic (CompWindow *w);
};

#define STATICDOCK_SCREEN(s)                                              \
StaticDock *sds = StaticDock::get (s)

class StaticDockWindow :
public PluginClassHandler<StaticDockWindow, CompWindow>,
public GLWindowInterface
{
public:
    StaticDockWindow (CompWindow *window);
    
    CompWindow *window;
    GLWindow   *gWindow;
    
    bool glPaint (const GLWindowPaintAttrib &attrib,
                  const GLMatrix            &matrix,
                  const CompRegion          &region,
                  unsigned int              mask);
};

#define STATICDOCK_WINDOW(w)                                               \
StaticDockWindow *sdw = StaticDockWindow::get (w)

class StaticDockPluginVTable :
public CompPlugin::VTableForScreenAndWindow<StaticDock,StaticDockWindow>
{
public:
    bool init ();
};

#endif /* _STATICDOCK_H */
