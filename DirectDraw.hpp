//
// File name: DirectDraw.hpp
//
// Description: The header file for a simple DirectDraw encapsulation.
//
// Author: John De Goes
//
// Project:
//
// Import libraries: Ddraw.lib
//
// Copyright (C) 1999 John De Goes -- All Rights Reserved
//

#ifndef __DIRECTDRAWHPP__
#define __DIRECTDRAWHPP__

#include <Stdio.H>
#include <Windows.H>
#include <DDraw.H>


bool PrintDirectDrawError ( HRESULT Error );
HRESULT WINAPI EnumModesCallback ( DDSURFACEDESC2 *SurfaceDesc, LPVOID AppData );

class DirectDrawSurface;

class DirectDrawManager {
   protected:
		LPDIRECTDRAW7 DirectDraw7;

		LONG PropWidth, PropHeight, PropBPP;
      bool FullScreen;

      bool ConnectToDirectDraw ();
	public:
		DirectDrawManager ();
		~DirectDrawManager ();

		bool CreateSurface ( DirectDrawSurface &Surface );

		bool SetDisplayMode ( LONG Width, LONG Height, LONG BPP );

		bool Initialize ( HWND Window );
		bool Uninitialize ();

      bool GetInterface ( LPDIRECTDRAW7 *Interface );
      bool GetBaseInterface ( LPDIRECTDRAW *Base );
};

class DirectDrawSurface {
   public:
      enum SurfaceType { Primary, Plain, Chain, Texture,
         ZBuffer, Alpha, Overlay, BumpMap, LightMap };

   protected:
      LPDIRECTDRAWSURFACE7 Surface7;

      bool TypeSet, Created, PropLum, PropAlpha,
           UseSourceColorKey, ShouldRepaint;

      LONG PropWidth,      PropHeight,       PropBPP,
           SurfWidth,      SurfHeight,       SurfPitch,
           PropChainCount;

      SurfaceType PropSurfaceType;

      friend class DirectDrawManager;

   public:
      DirectDrawSurface ();
      ~DirectDrawSurface ();

      bool StartAccess ( LPVOID *Pointer,
         RECT *Rect = NULL );
      bool EndAccess   ( RECT *Rect = NULL );

      bool SetSurfaceType ( SurfaceType Type );
      
      bool SetGeneralOptions ( LONG Width, LONG Height,
         LONG BPP );
      bool SetChainOptions ( LONG ChainCount );
      bool SetTextureOptions ( bool Alpha );
      bool SetBumpMapOptions ( bool Luminescence );

      LONG GetWidth  () { return SurfWidth;  }
      LONG GetHeight () { return SurfHeight; }
      LONG GetPitch  () { return SurfPitch;  }

      bool Show ();

      bool NeedsRepainting ();

      bool BlitTo ( DirectDrawSurface &Dest,
         RECT &DestRect );
      
      bool BlitPortionTo ( RECT &Portion,
         DirectDrawSurface &Dest, RECT &DestRect );
      
      bool BlitTo ( DirectDrawSurface &Dest,
         LONG DestX, LONG DestY );
      
      bool BlitPortionTo ( RECT &Portion,
         DirectDrawSurface &Dest, LONG DestX, LONG DestY );

      bool ClearToDepth ( DWORD Depth );
      bool ClearToColor ( DWORD Color );
      
      bool SetTransparentColorRange ( DWORD Color1,
         DWORD Color2 );

      bool GetInterface ( LPDIRECTDRAWSURFACE7 *Interface );

      bool GetBaseInterface ( LPDIRECTDRAWSURFACE *Base );
};

#endif