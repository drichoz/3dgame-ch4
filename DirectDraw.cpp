//
// File name: DirectDraw.cpp
//
// Description: The source for a simple DirectDraw
//              encapsulation.
//
// Author: John De Goes
//
// Project:
//
// Import libraries: Ddraw.lib
//
// Copyright (C) 1999 John De Goes -- All Rights Reserved
//

#include "DirectDraw.hpp"

DirectDrawManager::DirectDrawManager () {
   DirectDraw7 = NULL;
   FullScreen = false;

   // Establish connection to DirectDraw:
   ConnectToDirectDraw ();
}

DirectDrawManager::~DirectDrawManager () {
   DirectDraw7->Release ();
}

bool DirectDrawManager::ConnectToDirectDraw () {
	HRESULT Val;

   // Create a DirectDraw object:
   Val = DirectDrawCreateEx ( NULL,
      ( void ** ) &DirectDraw7,
      IID_IDirectDraw7, NULL );
   
	if ( FAILED ( Val ) ) {
		return PrintDirectDrawError ( Val );
	}

   // Successfully connected to DirectDraw

	return true;
}

bool DirectDrawManager::SetDisplayMode ( LONG Width,
        LONG Height, LONG BPP ) {
   bool CanSetMode = false;
   DDSURFACEDESC2 SurfaceDesc;

   // Set the display mode options to be used during
   // later initialization:

   FullScreen = false;

   PropWidth = Width; PropHeight = Height; PropBPP = BPP;

   SurfaceDesc.dwSize    = sizeof ( DDSURFACEDESC2 );
   SurfaceDesc.dwFlags   = DDSD_WIDTH | DDSD_HEIGHT;
   SurfaceDesc.dwWidth   = Width;
   SurfaceDesc.dwHeight  = Height;

   SurfaceDesc.ddpfPixelFormat.dwSize  =
      sizeof ( DDPIXELFORMAT );
   SurfaceDesc.ddpfPixelFormat.dwFlags = DDPF_RGB;

   SurfaceDesc.ddpfPixelFormat.dwRGBBitCount = BPP;

   // Make sure the specified display mode exists:
   DirectDraw7->EnumDisplayModes ( 0, &SurfaceDesc,
      &CanSetMode, ( LPDDENUMMODESCALLBACK2 )
      EnumModesCallback );

   if ( CanSetMode ) {
      FullScreen = true;
   }

   return FullScreen;
}

bool DirectDrawManager::Initialize ( HWND Window ) { 
   HRESULT Val;

   // Set the cooperative level and display mode (if
   // so requested):
   if ( FullScreen ) {
      Val = DirectDraw7->SetCooperativeLevel ( Window,
         DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN |
         DDSCL_ALLOWREBOOT );
      
      if ( FAILED ( Val ) )
         return PrintDirectDrawError ( Val );

      Val = DirectDraw7->SetDisplayMode ( PropWidth,
         PropHeight, PropBPP, 0, 0 );

      if ( FAILED ( Val ) )
         return PrintDirectDrawError ( Val );
   }
   else {
      Val = DirectDraw7->SetCooperativeLevel ( Window,
         DDSCL_NORMAL | DDSCL_ALLOWREBOOT ); 

      if ( FAILED ( Val ) )
         return PrintDirectDrawError ( Val );
   }

   return true;
}

bool DirectDrawManager::Uninitialize () {
   HRESULT Val;

   // If the display mode was set, restore it:
   if ( FullScreen ) {
      Val = DirectDraw7->RestoreDisplayMode ();
      
      if ( FAILED ( Val ) )
         return PrintDirectDrawError ( Val );
   }

   return true;
}

void SetColorBitDepth ( DDPIXELFORMAT &PF, LONG Depth,
        bool Alpha ) {

   // Set the bit depth for a color surface, optionally
   // with an alpha component.  Note that this function
   // uses common bit configurations, but there are others.

   ZeroMemory ( ( void * ) &PF, sizeof PF );
   PF.dwSize = sizeof ( DDPIXELFORMAT );
   
   switch ( Depth ) {
      case 8:
         PF.dwFlags        = DDPF_PALETTEINDEXED8;
         
         PF.dwRGBBitCount  = 8;
      break;
      case 15:
         PF.dwFlags        = DDPF_RGB;

         PF.dwRBitMask     = 0x1F << 10;
         PF.dwGBitMask     = 0x1F <<  5;
         PF.dwBBitMask     = 0x1F <<  0;
         
         PF.dwRGBBitCount  = 16;
      break;
      case 16:
         PF.dwFlags        = DDPF_RGB;

         if ( Alpha ) {
            PF.dwFlags |= DDPF_ALPHAPIXELS;

            PF.dwRBitMask        = 0x1F << 10;
            PF.dwGBitMask        = 0x1F << 5;
            PF.dwBBitMask        = 0x1F << 0;
            PF.dwRGBAlphaBitMask = 0x1  << 15;
         }
         else {
            PF.dwRBitMask     = 0x1F << 11;
            PF.dwGBitMask     = 0x3F << 5;
            PF.dwBBitMask     = 0x1F << 0;
         }
         
         PF.dwRGBBitCount  = 16;
      break;
      case 24:
         PF.dwFlags        = DDPF_RGB;

         PF.dwRBitMask     = 0xFF << 16;
         PF.dwGBitMask     = 0xFF << 8;
         PF.dwBBitMask     = 0xFF << 0;
         
         PF.dwRGBBitCount  = 24;
      break;
      case 32:
         PF.dwFlags        = DDPF_RGB;
         
         if ( Alpha ) {
            PF.dwFlags |= DDPF_ALPHAPIXELS;

            PF.dwRBitMask        = 0xFF << 16;
            PF.dwGBitMask        = 0xFF << 8;
            PF.dwBBitMask        = 0xFF << 0;
            PF.dwRGBAlphaBitMask = 0xFF << 24;
         }
         else {
            PF.dwRBitMask     = 0xFF << 16;
            PF.dwGBitMask     = 0xFF << 8;
            PF.dwBBitMask     = 0xFF << 0;
         }

         PF.dwRGBBitCount  = 32;
      break;
   }
}

void SetBumpMapBitDepth ( DDPIXELFORMAT &PF, LONG Depth,
        bool Light ) {

   // Set the pixel format for a bump map:

   ZeroMemory ( ( void * ) &PF, sizeof PF );
   PF.dwSize         = sizeof ( DDPIXELFORMAT );
   PF.dwFlags        = DDPF_BUMPDUDV;
   PF.dwBumpBitCount = Depth;

   switch ( Depth ) {
      // 15 and 8 bpp don't really make sense for bump maps,
      // so here they are ignored:
      case 16:
         if ( Light ) {
            PF.dwFlags |= DDPF_BUMPLUMINANCE;

            PF.dwBumpDuBitMask         = 0x1F << 0;
            PF.dwBumpDvBitMask         = 0x1F << 5;
            PF.dwBumpLuminanceBitMask  = 0x3F << 10;
         }
         else {
            PF.dwBumpDuBitMask         = 0xFF << 0;
            PF.dwBumpDvBitMask         = 0xFF << 8;
            PF.dwBumpLuminanceBitMask  = 0;
         }
      break;      
      case 24:
      case 32:
         if ( Light ) {
            PF.dwFlags |= DDPF_BUMPLUMINANCE;

            PF.dwBumpDuBitMask         = 0xFF << 0;
            PF.dwBumpDvBitMask         = 0xFF << 8;
            PF.dwBumpLuminanceBitMask  = 0xFF << 16;
         }
         else {
            PF.dwBumpDuBitMask         = 0xFF << 0;
            PF.dwBumpDvBitMask         = 0xFF << 8;
            PF.dwBumpLuminanceBitMask  = 0;
         }
      break;
   }
}

void SetAlphaBitDepth ( DDPIXELFORMAT &PF, LONG Depth ) {
   // Set the pixel format for an alpha surface:

   ZeroMemory ( ( void * ) &PF, sizeof PF );
   PF.dwSize          = sizeof ( DDPIXELFORMAT );
   PF.dwFlags         = DDPF_ALPHA;
   PF.dwAlphaBitDepth = Depth;
}

void SetZBufferBitDepth ( DDPIXELFORMAT &PF, LONG Depth ) {
   // Set the pixel format for a z-buffer:
   ZeroMemory ( ( void * ) &PF, sizeof PF );
   PF.dwSize         = sizeof ( DDPIXELFORMAT );
   PF.dwFlags        = DDPF_ZBUFFER;
   PF.dwZBufferBitDepth  = Depth;

   switch ( Depth ) {
      case 8:
         PF.dwZBitMask = 0x000000FF;
      break;
      case 15:
         PF.dwZBitMask = 0x00007FFF;
      break;
      case 16:
         PF.dwZBitMask = 0x0000FFFF;
      break;
      case 24:
         PF.dwZBitMask = 0x00FFFFFF;
      break;
      case 32:
         PF.dwZBitMask = 0xFFFFFFFF;
      break;
   }   
}

bool DirectDrawManager::CreateSurface (
        DirectDrawSurface &Surface ) {

   HRESULT Val;
   DDSURFACEDESC2 SurfaceDesc;
   DWORD EssentialCaps = 0, DesiredCaps = 0,
      EssentialCaps2 = 0, DesiredCaps2 = 0;

   if ( DirectDraw7 == NULL )
      return false;

   if ( !Surface.TypeSet )
      return false;

   if ( Surface.Created )
      return false;

   ZeroMemory ( &SurfaceDesc, sizeof ( DDSURFACEDESC2 ) );

   SurfaceDesc.dwSize  = sizeof ( DDSURFACEDESC2 );
   SurfaceDesc.dwFlags = DDSD_CAPS;

   switch ( Surface.PropSurfaceType ) {
      case DirectDrawSurface::Primary:
         // Create a primary surface with one backbuffer:

         SurfaceDesc.dwFlags |= DDSD_BACKBUFFERCOUNT;
         SurfaceDesc.dwBackBufferCount = 1; 
 
         EssentialCaps  |= DDSCAPS_COMPLEX | DDSCAPS_FLIP |
            DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE |
            DDSCAPS_VIDEOMEMORY;

         EssentialCaps2 |= 0;

         DesiredCaps    |= DDSCAPS_LIVEVIDEO |
            DDSCAPS_TEXTURE;

         DesiredCaps2   |= 0;
      break;

      case DirectDrawSurface::Chain:
         // Create a surface chain:

         SurfaceDesc.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT;
         SurfaceDesc.dwWidth  = Surface.PropWidth;
         SurfaceDesc.dwHeight = Surface.PropHeight;

         SurfaceDesc.dwFlags |= DDSD_PIXELFORMAT;
         
         SetColorBitDepth ( SurfaceDesc.ddpfPixelFormat,
            Surface.PropBPP, Surface.PropAlpha );

         SurfaceDesc.dwFlags |= DDSD_BACKBUFFERCOUNT;
         
         SurfaceDesc.dwBackBufferCount = 
            Surface.PropChainCount; 
 
         EssentialCaps  |= DDSCAPS_COMPLEX | DDSCAPS_FLIP |
            DDSCAPS_VIDEOMEMORY;

         EssentialCaps2 |= 0;

         DesiredCaps    |= DDSCAPS_LIVEVIDEO |
            DDSCAPS_TEXTURE | DDSCAPS_3DDEVICE;

         DesiredCaps2   |= 0;
      break;

      case DirectDrawSurface::Plain:
         // Create a plain surface:

         SurfaceDesc.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT;
         SurfaceDesc.dwWidth  = Surface.PropWidth;
         SurfaceDesc.dwHeight = Surface.PropHeight;
         
         SurfaceDesc.dwFlags |= DDSD_PIXELFORMAT;
         
         SetColorBitDepth ( SurfaceDesc.ddpfPixelFormat,
            Surface.PropBPP, Surface.PropAlpha );

         EssentialCaps  |= DDSCAPS_LIVEVIDEO |
            DDSCAPS_OFFSCREENPLAIN;
         
         EssentialCaps2 |= 0;

         DesiredCaps    |= DDSCAPS_VIDEOMEMORY |
            DDSCAPS_TEXTURE;
         
         DesiredCaps2   |= 0;
      break;

      case DirectDrawSurface::Texture:
         // Create a texture:

         SurfaceDesc.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT;
         SurfaceDesc.dwWidth  = Surface.PropWidth;
         SurfaceDesc.dwHeight = Surface.PropHeight;
         
         SurfaceDesc.dwFlags |= DDSD_PIXELFORMAT;
         SetColorBitDepth ( SurfaceDesc.ddpfPixelFormat,
            Surface.PropBPP, Surface.PropAlpha );

         SurfaceDesc.dwFlags       |= DDSD_TEXTURESTAGE;
         SurfaceDesc.dwTextureStage = 0;

         EssentialCaps  |= DDSCAPS_TEXTURE |
            DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;

         // With texture auto manage, we cannot specify
         // explicitly where the texture should be
         // located:
         EssentialCaps2 |= DDSCAPS2_TEXTUREMANAGE;

         DesiredCaps    |= DDSCAPS_LIVEVIDEO |
            DDSCAPS_3DDEVICE;

         DesiredCaps2   |= 0;
      break;

      case DirectDrawSurface::ZBuffer:
         // Create a z-buffer:

         SurfaceDesc.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT;
         SurfaceDesc.dwWidth  = Surface.PropWidth;
         SurfaceDesc.dwHeight = Surface.PropHeight;

         SurfaceDesc.dwFlags |= DDSD_PIXELFORMAT;
         SetZBufferBitDepth ( SurfaceDesc.ddpfPixelFormat,
            Surface.PropBPP );

         EssentialCaps  |= DDSCAPS_ZBUFFER;
         EssentialCaps2 |= 0;

         DesiredCaps    |= 0;
         DesiredCaps2   |= 0;
      break;

      case DirectDrawSurface::Alpha:
         // Create an alpha surface:

         SurfaceDesc.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT;
         SurfaceDesc.dwWidth  = Surface.PropWidth;
         SurfaceDesc.dwHeight = Surface.PropHeight;
         
         SurfaceDesc.dwFlags          |= DDSD_PIXELFORMAT |
            DDSD_ALPHABITDEPTH;

         SurfaceDesc.dwAlphaBitDepth   = Surface.PropBPP;
         
         SetAlphaBitDepth ( SurfaceDesc.ddpfPixelFormat,
            Surface.PropBPP );

         EssentialCaps  |= DDSCAPS_ALPHA;
         EssentialCaps2 |= 0;

         DesiredCaps    |= DDSCAPS_VIDEOMEMORY;
         DesiredCaps2   |= 0;
      break;

      case DirectDrawSurface::Overlay:
         // Create an overlay:

         SurfaceDesc.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT;
         SurfaceDesc.dwWidth  = Surface.PropWidth;
         SurfaceDesc.dwHeight = Surface.PropHeight;
         
         SurfaceDesc.dwFlags |= DDSD_PIXELFORMAT;
         SetColorBitDepth ( SurfaceDesc.ddpfPixelFormat,
            Surface.PropBPP, Surface.PropAlpha );

         EssentialCaps  |= DDSCAPS_OVERLAY;
         EssentialCaps2 |= 0;
         
         DesiredCaps    |= DDSCAPS_VIDEOMEMORY |
            DDSCAPS_LIVEVIDEO | DDSCAPS_3DDEVICE |
            DDSCAPS_TEXTURE;

         DesiredCaps2   |= 0;
      break;

      case DirectDrawSurface::BumpMap:
         // Create a bump map:

         SurfaceDesc.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT;
         SurfaceDesc.dwWidth  = Surface.PropWidth;
         SurfaceDesc.dwHeight = Surface.PropHeight;
         
         SurfaceDesc.dwFlags |= DDSD_PIXELFORMAT;
         SetBumpMapBitDepth ( SurfaceDesc.ddpfPixelFormat,
            Surface.PropBPP, Surface.PropLum );

         SurfaceDesc.dwFlags       |= DDSD_TEXTURESTAGE;
         
         // WARNING: Code assumes bump map blending occurs
         // at the second stage. The environment map should
         // be located in the third stage.
         SurfaceDesc.dwTextureStage = 1;

         EssentialCaps  |= DDSCAPS_TEXTURE;
         EssentialCaps2 |= 0;

         DesiredCaps    |= DDSCAPS_LIVEVIDEO |
            DDSCAPS_3DDEVICE;

         DesiredCaps2   |= 0;
      break;

      case DirectDrawSurface::LightMap:
         // Create an RGB light map:

		   SurfaceDesc.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT;
         SurfaceDesc.dwWidth  = Surface.PropWidth;
         SurfaceDesc.dwHeight = Surface.PropHeight;
         
         SurfaceDesc.dwFlags |= DDSD_PIXELFORMAT;
         SetColorBitDepth ( SurfaceDesc.ddpfPixelFormat,
            Surface.PropBPP, false );

         SurfaceDesc.dwFlags       |= DDSD_TEXTURESTAGE;
         
         // WARNING: Code assumes light map blending occurs
         // at the second stage (this means bump mapping
         // and light mapping are incompatible operations,
         // since the above bump mapping code also uses
         // the second stage; change one stage number or
         // the other to fix this).
         SurfaceDesc.dwTextureStage = 1;

         EssentialCaps  |= DDSCAPS_TEXTURE;
         EssentialCaps2 |= DDSCAPS2_TEXTUREMANAGE;

         DesiredCaps    |= 0;
         DesiredCaps2   |= 0;
      break;
   }

   SurfaceDesc.ddsCaps.dwCaps  = EssentialCaps  |
      DesiredCaps;

   SurfaceDesc.ddsCaps.dwCaps2 = EssentialCaps2 |
      DesiredCaps2;

   // Try to get the most capabilities:
   if ( DirectDraw7->CreateSurface ( &SurfaceDesc,
        &Surface.Surface7, NULL ) != DD_OK ) {

      SurfaceDesc.ddsCaps.dwCaps  = EssentialCaps;
      
      SurfaceDesc.ddsCaps.dwCaps2 = EssentialCaps2 |
         DesiredCaps2;

      Val = DirectDraw7->CreateSurface ( &SurfaceDesc,
         &Surface.Surface7, NULL );

      if ( FAILED ( Val ) ) {
         SurfaceDesc.ddsCaps.dwCaps  = EssentialCaps  |
            DesiredCaps;

         SurfaceDesc.ddsCaps.dwCaps2 = EssentialCaps2;
         
         Val = DirectDraw7->CreateSurface ( &SurfaceDesc,
            &Surface.Surface7, NULL ); 
         
         if ( FAILED ( Val ) ) {
            SurfaceDesc.ddsCaps.dwCaps  = EssentialCaps;
            SurfaceDesc.ddsCaps.dwCaps2 = EssentialCaps2;
         
            Val = DirectDraw7->CreateSurface ( &SurfaceDesc,
               &Surface.Surface7, NULL );

            if ( FAILED ( Val ) ) {
               return PrintDirectDrawError ( Val );
            }
         }
      }
   }

   Surface.Created = true;

   // Grab the width, height, and pitch of the new surface:
   ZeroMemory ( ( void * ) &SurfaceDesc,
      sizeof ( DDSURFACEDESC2 ) );

   SurfaceDesc.dwSize = sizeof ( DDSURFACEDESC2 );

   Surface.Surface7->GetSurfaceDesc ( &SurfaceDesc );

   Surface.SurfWidth  = SurfaceDesc.dwWidth;
   Surface.SurfHeight = SurfaceDesc.dwHeight;
   Surface.SurfPitch  = SurfaceDesc.lPitch;

   return true;
}

bool DirectDrawManager::GetInterface (
        LPDIRECTDRAW7 *Interface ) {

   if ( DirectDraw7 == NULL )
      return false;

   ( *Interface ) = DirectDraw7;

   ( *Interface )->AddRef ();

   return true;
}

bool DirectDrawManager::GetBaseInterface (
        LPDIRECTDRAW *Base ) {

   // Retrieve the base interface:
   if ( DirectDraw7 == NULL )
      return false;

   return ( DirectDraw7->QueryInterface ( IID_IDirectDraw,
      ( void ** ) Base ) == S_OK );
}

DirectDrawSurface::DirectDrawSurface () {
   ShouldRepaint = UseSourceColorKey = TypeSet = Created = false;
   PropChainCount = 0;
   PropLum = PropAlpha = false;
   SurfWidth = SurfHeight = SurfPitch = 0;   
}

DirectDrawSurface::~DirectDrawSurface () {
   if ( Created )
      Surface7->Release ();
}

bool DirectDrawSurface::StartAccess ( LPVOID *Pointer,
        RECT *Rect ) {

   LPDIRECTDRAWSURFACE7 Backbuffer;
   DDSURFACEDESC2       SurfaceDesc;
   DDSCAPS2             SurfaceCaps;
   HRESULT              Val;

   // Obtain a pointer to the surface's memory:

   ZeroMemory ( &SurfaceDesc, sizeof ( DDSURFACEDESC2 ) );

   SurfaceDesc.dwSize = sizeof ( DDSURFACEDESC2 );

   if ( !Created )
      return false;

   if ( Surface7->IsLost () != DD_OK ) {
      if ( FAILED ( Surface7->Restore () ) )
         return false;

      ShouldRepaint = true;
   }

   // If the surface is a primary surface, make sure we
   // obtain a pointer to the backbuffer, since that is the
   // surface the application probably wants to write to:
   if ( PropSurfaceType == Primary ) {

      SurfaceCaps.dwCaps = DDSCAPS_BACKBUFFER;

      Val = Surface7->GetAttachedSurface ( &SurfaceCaps,
         &Backbuffer );

      if ( FAILED ( Val ) )
         return PrintDirectDrawError ( Val );

      Val = Backbuffer->Lock ( Rect, &SurfaceDesc,
         DDLOCK_NOSYSLOCK | DDLOCK_WAIT, NULL );
   }
   else {
      Val = Surface7->Lock ( Rect, &SurfaceDesc,
         DDLOCK_NOSYSLOCK | DDLOCK_WAIT, NULL );
   }

   if ( FAILED ( Val ) )
      return PrintDirectDrawError ( Val );

   ( *Pointer ) = SurfaceDesc.lpSurface;

   return true;
}

bool DirectDrawSurface::EndAccess ( RECT *Rect ) {
   LPDIRECTDRAWSURFACE7 Backbuffer;
   DDSCAPS2             SurfaceCaps;
   HRESULT              Val;

   // End access to the surface:

   if ( !Created )
      return false;

   if ( PropSurfaceType == Primary ) {
      SurfaceCaps.dwCaps = DDSCAPS_BACKBUFFER;

      Val = Surface7->GetAttachedSurface ( &SurfaceCaps,
         &Backbuffer );

      if ( FAILED ( Val ) )
         return PrintDirectDrawError ( Val );

      Val = Backbuffer->Unlock ( Rect );
   }
   else {
      Val = Surface7->Unlock ( Rect );
   }

   if ( FAILED ( Val ) )
      return PrintDirectDrawError ( Val );

   return true;
}

bool DirectDrawSurface::SetSurfaceType (
        SurfaceType Type ) {

   if ( Created )
      return false;

   PropSurfaceType = Type;

   TypeSet = true;

   return true;
}

bool DirectDrawSurface::SetGeneralOptions ( LONG Width,
        LONG Height, LONG BPP ) {

   if ( Created )
      return false;

   PropWidth         = Width;
   PropHeight        = Height;
   PropBPP           = BPP;

   return true;
}

bool DirectDrawSurface::SetChainOptions (
        LONG ChainCount ) {

   if ( Created )
      return false;

   PropChainCount = ChainCount;

   return true;
}

bool DirectDrawSurface::SetTextureOptions ( bool Alpha ) {
   if ( Created )
      return false;

   PropAlpha = Alpha;

   return true;
}

bool DirectDrawSurface::SetBumpMapOptions (
        bool Luminescence ) {

   if ( Created )
      return false;

   PropLum = Luminescence;

   return true;
}

bool DirectDrawSurface::Show () {
   HRESULT Val;

   // Display the backbuffer of a primary surface:

   if ( !Created )
      return false;

   if ( PropSurfaceType != Primary )
      return false;

   if ( Surface7->IsLost () != DD_OK ) {
      if ( FAILED ( Surface7->Restore () ) )
         return false;

      ShouldRepaint = true;
   }

   Val = Surface7->Flip ( NULL, DDFLIP_WAIT );

   if ( FAILED ( Val ) )
      return PrintDirectDrawError ( Val );

   return true;
}

bool DirectDrawSurface::BlitTo ( DirectDrawSurface &Dest,
        RECT &DestRect ) {

   RECT Portion;

   if ( !Created )
      return false;

   Portion.left = 0; Portion.top = 0;
   Portion.right = PropWidth; Portion.bottom = PropHeight;

   return BlitPortionTo ( Portion, Dest, DestRect );
}

bool DirectDrawSurface::BlitPortionTo ( RECT &Portion,
        DirectDrawSurface &Dest, RECT &DestRect ) {

   DDBLTFX BlitFX;
   DWORD Flags = DDBLT_WAIT;
   HRESULT Val;

   if ( !Created )
      return false;

   if ( Surface7->IsLost () != DD_OK ) {
      if ( FAILED ( Surface7->Restore () ) )
         return false;

      ShouldRepaint = true;
   }

   ZeroMemory ( ( void * ) &BlitFX, sizeof ( DDBLTFX ) );
   BlitFX.dwSize = sizeof ( DDBLTFX );

   if ( UseSourceColorKey )
      Flags |= DDBLT_KEYSRC;

   Val = Dest.Surface7->Blt ( &DestRect, this->Surface7,
      &Portion, Flags, &BlitFX );
   
   if ( FAILED ( Val ) )
      return PrintDirectDrawError ( Val );

   return true;
}

bool DirectDrawSurface::BlitTo ( DirectDrawSurface &Dest,
        LONG DestX, LONG DestY ) {

   RECT DestRect;

   DestRect.left     = DestX;
   DestRect.right    = DestX + PropWidth;
   DestRect.top      = DestY;
   DestRect.bottom   = DestY + PropHeight;

   return BlitTo ( Dest, DestRect );
}

bool DirectDrawSurface::BlitPortionTo ( RECT &Portion,
        DirectDrawSurface &Dest, LONG DestX, LONG DestY ) {

   RECT DestRect;

   DestRect.left     = DestX;
   DestRect.right    = DestX + PropWidth;
   DestRect.top      = DestY;
   DestRect.bottom   = DestY + PropHeight;

   return BlitPortionTo ( Portion, Dest, DestRect );
}

bool DirectDrawSurface::ClearToDepth ( DWORD Depth ) {
   DDBLTFX BlitFX;
   RECT Portion;
   DWORD Flags = 0;
   HRESULT Val;

   if ( !Created )
      return false;

   if ( PropSurfaceType != ZBuffer )
      return false;

   if ( Surface7->IsLost () != DD_OK ) {
      if ( FAILED ( Surface7->Restore () ) )
         return false;

      ShouldRepaint = true;
   }

   // Clear z-buffer to a specific depth:

   Portion.left = 0; Portion.top = 0;
   Portion.right = PropWidth; Portion.bottom = PropHeight;

   ZeroMemory ( ( void * ) &BlitFX, sizeof ( DDBLTFX ) );
   BlitFX.dwSize = sizeof ( DDBLTFX );

   Flags |= DDBLT_DEPTHFILL;

   BlitFX.dwFillDepth = Depth;

   Val = Surface7->Blt ( NULL, NULL, &Portion, Flags,
      &BlitFX );

   if ( FAILED ( Val ) )
      return PrintDirectDrawError ( Val );

   return true;
}

bool DirectDrawSurface::ClearToColor ( DWORD Color ) {
   DDBLTFX BlitFX;
   RECT Portion;
   DWORD Flags = 0;
   HRESULT Val;

   if ( !Created )
      return false;

   if ( Surface7->IsLost () != DD_OK ) {
      if ( FAILED ( Surface7->Restore () ) )
         return false;

      ShouldRepaint = true;
   }

   // Clear surface to a specific color:

   Portion.left = 0; Portion.top = 0;
   Portion.right = PropWidth; Portion.bottom = PropHeight;

   ZeroMemory ( ( void * ) &BlitFX, sizeof ( DDBLTFX ) );
   BlitFX.dwSize = sizeof ( DDBLTFX );

   Flags |= DDBLT_COLORFILL;

   if ( !PropAlpha ) {
      BlitFX.dwFillColor = Color;
   }
   else {
      BlitFX.dwFillPixel = Color;
   }

   Val = Surface7->Blt ( NULL, NULL, &Portion, Flags,
      &BlitFX );

   if ( FAILED ( Val ) )
      return PrintDirectDrawError ( Val );

   return true;
}
      
bool DirectDrawSurface::SetTransparentColorRange (
        DWORD Color1, DWORD Color2 ) {

   DWORD Flags = DDCKEY_COLORSPACE;
   DDCOLORKEY ColorKey;
   HRESULT Val;

   // Set the source color key range for the surface:

   if ( !Created )
      return false;

   if ( PropSurfaceType == Overlay ) {
      Flags |= DDCKEY_SRCOVERLAY;
   }
   else Flags |= DDCKEY_SRCBLT;

   ColorKey.dwColorSpaceLowValue  = Color1; 
   ColorKey.dwColorSpaceHighValue = Color2; 

   Val = Surface7->SetColorKey ( Flags, &ColorKey );

   if ( FAILED ( Val ) )
      PrintDirectDrawError ( Val );

   UseSourceColorKey = true;

   return true;
}

bool DirectDrawSurface::NeedsRepainting () {
   if ( ShouldRepaint ) {
      ShouldRepaint = false;
      return true;
   }

   return false;
}

bool DirectDrawSurface::GetInterface (
        LPDIRECTDRAWSURFACE7 *Interface ) {

   if ( Surface7 == NULL )
      return false;
   
   ( *Interface ) = Surface7;

   ( *Interface )->AddRef ();

   return true;
}

bool DirectDrawSurface::GetBaseInterface (
        LPDIRECTDRAWSURFACE *Base ) {

   return ( Surface7->QueryInterface (
               IID_IDirectDrawSurface,
               ( void ** ) Base ) == S_OK );
}

HRESULT WINAPI EnumModesCallback (
      DDSURFACEDESC2 *SurfaceDesc, LPVOID AppData ) {

   bool *CanSetMode = ( bool * ) ( AppData );
   
   // Possibly check surface description here for best
   // pixel configuration...

   ( *CanSetMode ) = true;

   return DDENUMRET_CANCEL;
}

bool FatalError ( TCHAR *Message ) {
   TCHAR ErrorMessage [ 200 ];
   
   sprintf ( ErrorMessage, "DirectDraw Error: %s",
      Message );

   MessageBox ( GetActiveWindow (), ErrorMessage,
      "DirectDraw Fatal Error", MB_OK | MB_ICONERROR );

   return false;
}

bool PrintDirectDrawError ( HRESULT Error ) {
   switch ( Error ) {
      case DD_OK:
         FatalError ( "DD_OK" );
      break;
      case DDERR_ALREADYINITIALIZED:
         FatalError ( "DDERR_ALREADYINITIALIZED" );
      break;
      case DDERR_BLTFASTCANTCLIP:
         FatalError ( "DDERR_BLTFASTCANTCLIP" );
      break;
      case DDERR_CANNOTATTACHSURFACE:
         FatalError ( "DDERR_CANNOTATTACHSURFACE" );
      break;
      case DDERR_CANNOTDETACHSURFACE:
         FatalError ( "DDERR_CANNOTDETACHSURFACE" );
      break;
      case DDERR_CANTCREATEDC:
         FatalError ( "DDERR_CANTCREATEDC" );
      break;
      case DDERR_CANTDUPLICATE:
         FatalError ( "DDERR_CANTDUPLICATE" );
      break;
      case DDERR_CANTLOCKSURFACE:
         FatalError ( "DDERR_CANTLOCKSURFACE" );
      break;
      case DDERR_CANTPAGELOCK:
         FatalError ( "DDERR_CANTPAGELOCK" );
      break;
      case DDERR_CANTPAGEUNLOCK:
         FatalError ( "DDERR_CANTPAGEUNLOCK" );
      break;
      case DDERR_CLIPPERISUSINGHWND:
         FatalError ( "DDERR_CLIPPERISUSINGHWND" );
      break;
      case DDERR_COLORKEYNOTSET:
         FatalError ( "DDERR_COLORKEYNOTSET" );
      break;
      case DDERR_CURRENTLYNOTAVAIL:
         FatalError ( "DDERR_CURRENTLYNOTAVAIL" );
      break;
      case DDERR_DCALREADYCREATED:
         FatalError ( "DDERR_DCALREADYCREATED" );
      break;
      case DDERR_DEVICEDOESNTOWNSURFACE:
         FatalError ( "DDERR_DEVICEDOESNTOWNSURFACE" );
      break;
      case DDERR_DIRECTDRAWALREADYCREATED:
         FatalError ( "DDERR_DIRECTDRAWALREADYCREATED" );
      break;
      case DDERR_EXCEPTION:
         FatalError ( "DDERR_EXCEPTION" );
      break;
      case DDERR_EXCLUSIVEMODEALREADYSET:
         FatalError ( "DDERR_EXCLUSIVEMODEALREADYSET" );
      break;
      case DDERR_EXPIRED:
         FatalError ( "DDERR_EXPIRED" );
      break;
      case DDERR_GENERIC:
         FatalError ( "DDERR_GENERIC" );
      break;
      case DDERR_HEIGHTALIGN:
         FatalError ( "DDERR_HEIGHTALIGN" );
      break;
      case DDERR_HWNDALREADYSET:
         FatalError ( "DDERR_HWNDALREADYSET" );
      break;
      case DDERR_HWNDSUBCLASSED:
         FatalError ( "DDERR_HWNDSUBCLASSED" );
      break;
      case DDERR_IMPLICITLYCREATED:
         FatalError ( "DDERR_IMPLICITLYCREATED" );
      break;
      case DDERR_INCOMPATIBLEPRIMARY:
         FatalError ( "DDERR_INCOMPATIBLEPRIMARY" );
      break;
      case DDERR_INVALIDCAPS:
         FatalError ( "DDERR_INVALIDCAPS" );
      break;
      case DDERR_INVALIDCLIPLIST:
         FatalError ( "DDERR_INVALIDCLIPLIST" );
      break;
      case DDERR_INVALIDDIRECTDRAWGUID:
         FatalError ( "DDERR_INVALIDDIRECTDRAWGUID" );
      break;
      case DDERR_INVALIDMODE:
         FatalError ( "DDERR_INVALIDMODE" );
      break;
      case DDERR_INVALIDOBJECT:
         FatalError ( "DDERR_INVALIDOBJECT" );
      break;
      case DDERR_INVALIDPARAMS:
         FatalError ( "DDERR_INVALIDPARAMS" );
      break;
      case DDERR_INVALIDPIXELFORMAT:
         FatalError ( "DDERR_INVALIDPIXELFORMAT" );
      break;
      case DDERR_INVALIDPOSITION:
         FatalError ( "DDERR_INVALIDPOSITION" );
      break;
      case DDERR_INVALIDRECT:
         FatalError ( "DDERR_INVALIDRECT" );
      break;
      case DDERR_INVALIDSTREAM:
         FatalError ( "DDERR_INVALIDSTREAM" );
      break;
      case DDERR_INVALIDSURFACETYPE:
         FatalError ( "DDERR_INVALIDSURFACETYPE" );
      break;
      case DDERR_LOCKEDSURFACES:
         FatalError ( "DDERR_LOCKEDSURFACES" );
      break;
      case DDERR_MOREDATA:
         FatalError ( "DDERR_MOREDATA" );
      break;
      case DDERR_NO3D:
         FatalError ( "DDERR_NO3D" );
      break;
      case DDERR_NOALPHAHW:
         FatalError ( "DDERR_NOALPHAHW" );
      break;
      case DDERR_NOBLTHW:
         FatalError ( "DDERR_NOBLTHW" );
      break;
      case DDERR_NOCLIPLIST:
         FatalError ( "DDERR_NOCLIPLIST" );
      break;
      case DDERR_NOCLIPPERATTACHED:
         FatalError ( "DDERR_NOCLIPPERATTACHED" );
      break;
      case DDERR_NOCOLORCONVHW:
         FatalError ( "DDERR_NOCOLORCONVHW" );
      break;
      case DDERR_NOCOLORKEY:
         FatalError ( "DDERR_NOCOLORKEY" );
      break;
      case DDERR_NOCOLORKEYHW:
         FatalError ( "DDERR_NOCOLORKEYHW" );
      break;
      case DDERR_NOCOOPERATIVELEVELSET:
         FatalError ( "DDERR_NOCOOPERATIVELEVELSET" );
      break;
      case DDERR_NODC:
         FatalError ( "DDERR_NODC" );
      break;
      case DDERR_NODDROPSHW:
         FatalError ( "DDERR_NODDROPSHW" );
      break;
      case DDERR_NODIRECTDRAWHW:
         FatalError ( "DDERR_NODIRECTDRAWHW" );
      break;
      case DDERR_NODIRECTDRAWSUPPORT:
         FatalError ( "DDERR_NODIRECTDRAWSUPPORT" );
      break;
      case DDERR_NOEMULATION:
         FatalError ( "DDERR_NOEMULATION" );
      break;
      case DDERR_NOEXCLUSIVEMODE:
         FatalError ( "DDERR_NOEXCLUSIVEMODE" );
      break;
      case DDERR_NOFLIPHW:
         FatalError ( "DDERR_NOFLIPHW" );
      break;
      case DDERR_NOFOCUSWINDOW:
         FatalError ( "DDERR_NOFOCUSWINDOW" );
      break;
      case DDERR_NOGDI:
         FatalError ( "DDERR_NOGDI" );
      break;
      case DDERR_NOHWND:
         FatalError ( "DDERR_NOHWND" );
      break;
      case DDERR_NOMIPMAPHW:
         FatalError ( "DDERR_NOMIPMAPHW" );
      break;
      case DDERR_NOMIRRORHW:         
         FatalError ( "DDERR_NOMIRRORHW" );
      break;
      case DDERR_NONONLOCALVIDMEM:         
         FatalError ( "DDERR_NONONLOCALVIDMEM" );
      break;
      case DDERR_NOOPTIMIZEHW:
         FatalError ( "DDERR_NOOPTIMIZEHW" );
      break;
      case DDERR_NOOVERLAYDEST:
         FatalError ( "DDERR_NOOVERLAYDEST" );
      break;
      case DDERR_NOOVERLAYHW:
         FatalError ( "DDERR_NOOVERLAYHW" );
      break;
      case DDERR_NOPALETTEATTACHED:
         FatalError ( "DDERR_NOPALETTEATTACHED" );
      break;
      case DDERR_NOPALETTEHW:
         FatalError ( "DDERR_NOPALETTEHW" );
      break;
      case DDERR_NORASTEROPHW:
         FatalError ( "DDERR_NORASTEROPHW" );
      break;
      case DDERR_NOROTATIONHW:
         FatalError ( "DDERR_NOROTATIONHW" );
      break;
      case DDERR_NOSTRETCHHW:
         FatalError ( "DDERR_NOSTRETCHHW" );
      break;
      case DDERR_NOT4BITCOLOR:
         FatalError ( "DDERR_NOT4BITCOLOR" );
      break;
      case DDERR_NOT4BITCOLORINDEX:
         FatalError ( "DDERR_NOT4BITCOLORINDEX" );
      break;
      case DDERR_NOT8BITCOLOR:
         FatalError ( "DDERR_NOT8BITCOLOR" );
      break;
      case DDERR_NOTAOVERLAYSURFACE:
         FatalError ( "DDERR_NOTAOVERLAYSURFACE" );
      break;
      case DDERR_NOTEXTUREHW:
         FatalError ( "DDERR_NOTEXTUREHW" );
      break;
      case DDERR_NOTFLIPPABLE:
         FatalError ( "DDERR_NOTFLIPPABLE" );
      break;
      case DDERR_NOTFOUND:
         FatalError ( "DDERR_NOTFOUND" );
      break;
      case DDERR_NOTINITIALIZED:
         FatalError ( "DDERR_NOTINITIALIZED" );
      break;
      case DDERR_NOTLOADED:
         FatalError ( "DDERR_NOTLOADED" );
      break;
      case DDERR_NOTLOCKED:
         FatalError ( "DDERR_NOTLOCKED" );
      break;
      case DDERR_NOTPAGELOCKED:
         FatalError ( "DDERR_NOTPAGELOCKED" );
      break;
      case DDERR_NOTPALETTIZED:
         FatalError ( "DDERR_NOTPALETTIZED" );
      break;
      case DDERR_NOVSYNCHW:
         FatalError ( "DDERR_NOVSYNCHW" );
      break;
      case DDERR_NOZBUFFERHW:
         FatalError ( "DDERR_NOZBUFFERHW" );
      break;
      case DDERR_NOZOVERLAYHW:
         FatalError ( "DDERR_NOZOVERLAYHW" );
      break;
      case DDERR_OUTOFCAPS:
         FatalError ( "DDERR_OUTOFCAPS" );
      break;
      case DDERR_OUTOFMEMORY:
         FatalError ( "DDERR_OUTOFMEMORY" );
      break;
      case DDERR_OUTOFVIDEOMEMORY:
         FatalError ( "DDERR_OUTOFVIDEOMEMORY" );
      break;
      case DDERR_OVERLAPPINGRECTS:
         FatalError ( "DDERR_OVERLAPPINGRECTS" );
      break;
      case DDERR_OVERLAYCANTCLIP:
         FatalError ( "DDERR_OVERLAYCANTCLIP" );
      break;
      case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:
         FatalError ("DDERR_OVERLAYCOLORKEYONLYONEACTIVE");
      break;
      case DDERR_OVERLAYNOTVISIBLE:
         FatalError ( "DDERR_OVERLAYNOTVISIBLE" );
      break;
      case DDERR_PALETTEBUSY:
         FatalError ( "DDERR_PALETTEBUSY" );
      break;
      case DDERR_PRIMARYSURFACEALREADYEXISTS:
         FatalError ("DDERR_PRIMARYSURFACEALREADYEXISTS");
      break;
      case DDERR_REGIONTOOSMALL:
         FatalError ( "DDERR_REGIONTOOSMALL" );
      break;
      case DDERR_SURFACEALREADYATTACHED:
         FatalError ( "DDERR_SURFACEALREADYATTACHED" );
      break;
      case DDERR_SURFACEALREADYDEPENDENT:
         FatalError ( "DDERR_SURFACEALREADYDEPENDENT" );
      break;
      case DDERR_SURFACEBUSY:
         FatalError ( "DDERR_SURFACEBUSY" );
      break;
      case DDERR_SURFACEISOBSCURED:
         FatalError ( "DDERR_SURFACEISOBSCURED" );
      break;
      case DDERR_SURFACELOST:
         FatalError ( "DDERR_SURFACELOST" );
      break;
      case DDERR_SURFACENOTATTACHED:
         FatalError ( "DDERR_SURFACENOTATTACHED" );
      break;
      case DDERR_TOOBIGHEIGHT:
         FatalError ( "DDERR_TOOBIGHEIGHT" );
      break;
      case DDERR_TOOBIGSIZE:
         FatalError ( "DDERR_TOOBIGSIZE" );
      break;
      case DDERR_TOOBIGWIDTH:
         FatalError ( "DDERR_TOOBIGWIDTH" );
      break;
      case DDERR_UNSUPPORTED:
         FatalError ( "DDERR_UNSUPPORTED" );
      break;
      case DDERR_UNSUPPORTEDFORMAT:
         FatalError ( "DDERR_UNSUPPORTEDFORMAT" );
      break;
      case DDERR_UNSUPPORTEDMASK:
         FatalError ( "DDERR_UNSUPPORTEDMASK" );
      break;
      case DDERR_UNSUPPORTEDMODE:
         FatalError ( "DDERR_UNSUPPORTEDMODE" );
      break;
      case DDERR_VERTICALBLANKINPROGRESS:
         FatalError ( "DDERR_VERTICALBLANKINPROGRESS" );
      break;
      case DDERR_VIDEONOTACTIVE:
         FatalError ( "DDERR_VIDEONOTACTIVE" );
      break;
      case DDERR_WASSTILLDRAWING:
         FatalError ( "DDERR_WASSTILLDRAWING" );
      break;
      case DDERR_WRONGMODE:
         FatalError ( "DDERR_WRONGMODE" );
      break;
      case DDERR_XALIGN:
         FatalError ( "DDERR_XALIGN" );
      break;
   }
   return false;
}
