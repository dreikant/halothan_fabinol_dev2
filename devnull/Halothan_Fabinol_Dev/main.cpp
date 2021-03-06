#include <windows.h>
#include <stdlib.h>

#include "Detours.h"

#define _DEBUG_LOG    TRUE
#include "LogFile.h"

//#include "BF3_SDK.h"
#include "bf3_sdk_helper.h"

//typedef HRESULT (WINAPI* tPresent)(LPDIRECT3DSWAPCHAIN9 pSwapChain, const RECT *pSourceRect,const RECT *pDestRect,HWND hDestWindowOverride,const RGNDATA *pDirtyRegion,DWORD dwFlags);
//tPresent oPresent;

typedef HRESULT (WINAPI* tPresent)(IDXGISwapChain* theClass, unsigned int, unsigned int);
tPresent oPresent;

static fb::Vec3 Vec3Transform( fb::Vec3 In, fb::LinearTransform * pm )
{
	fb::Vec3 Out;

	Out.x = pm->left.x * In.x + pm->up.x * In.y + pm->forward.x * In.z + pm->trans.x;
	Out.y = pm->left.y * In.x + pm->up.y * In.y + pm->forward.y * In.z + pm->trans.y;
	Out.z = pm->left.z * In.x + pm->up.z * In.y + pm->forward.z * In.z + pm->trans.z;

	return Out;
}

void drawESP()
{
	    for ( unsigned int i = 0; i <= 64; i ++ )
        {
            fb::ClientPlayer* localPlayer;
            fb::ClientPlayer* targetPlayer;
			BOSS* cheats = &BOSS();

            localPlayer  = cheats->getLocalPlayer( );
            targetPlayer = cheats->getPlayerById( i );

			if ( VALID( localPlayer ) && VALID( targetPlayer ) && localPlayer != targetPlayer )
            {
                fb::ClientSoldierEntity* localSoldier;
                fb::ClientSoldierEntity* targetSoldier;

                localSoldier  = cheats->getSoldier( localPlayer );
                targetSoldier = cheats->getSoldier( targetPlayer );

                if ( VALID( localSoldier ) && VALID( targetSoldier ) )
                {
                    fb::DebugRenderer2* engineRender = fb::DebugRenderer2::Singleton( );

                    if ( VALID( engineRender ) )
                    {
                        float screenX, screenY;
						engineRender->drawText( 5, i*20, fb::Color32( 255, 0, 255, 255 ), targetPlayer->m_name.GetString( ), 1 );
						if( VALID(targetSoldier->m_characterPhysicsentity) )
						{
							fb::AxisAlignedBox AABB = targetSoldier->m_characterPhysicsentity->m_collisionShapes->m_aabbs[0]; 
							fb::AxisAlignedBox R;

							R.min = Vec3Transform( AABB.min, targetSoldier->m_characterPhysicsentity->m_gameWorldTransform);

							engineRender->drawSphere(R.min,5,fb::Color32(0,0,255,255));
						}
						/*						
                        if ( cheats->worldToScreen( targetSoldier, &screenX, &screenY ) )
                        {
                            engineRender->drawText( screenX, screenY, fb::Color32( 255, 0, 255, 255 ), targetPlayer->m_name.GetString( ), 1 );
                        }*/
                    }

                }
            }
        }  
}

//HRESULT WINAPI hkPresent(LPDIRECT3DSWAPCHAIN9 pSwapChain, const RECT *pSourceRect,const RECT *pDestRect,HWND hDestWindowOverride,const RGNDATA *pDirtyRegion,DWORD dwFlags)
HRESULT WINAPI hkPresent(IDXGISwapChain* theclass, unsigned int d, unsigned int e)
{
	fb::DebugRenderer2* engineRender = fb::DebugRenderer2::Singleton( );

	if( VALID(engineRender) )
	{
		//engineRender->drawText(5,5,fb::Color32(255,0,0,255),"Test",1);
		drawESP();
	}
	return oPresent(theclass, d, e);
}

DWORD CreateDetours()
{
	// init hooks.
	CLogFile logFile = CLogFile("log.txt",true);
	logFile.Write("Create Detours...");
	logFile.Write("Create Detours Finished...");
	fb::DxRenderer* g_dxRenderer = fb::DxRenderer::Singleton( );
	// Test Kommentar
	//insekt du sau
	if( g_dxRenderer == NULL )
	{
		logFile.Write("Renderer not found...");
	}else
	{
		logFile.Write("Swapchain found %X ... ",g_dxRenderer->pSwapChain);

		// try swapchain detour.
		if ( VALID( g_dxRenderer->pSwapChain ) )
		{
			DWORD* vtable    = ( DWORD* )g_dxRenderer->pSwapChain;
			vtable            = ( DWORD* )vtable[0];

			// ALTERNATIVE:
			DWORD dxgi_base = (DWORD)GetModuleHandle("dxgi.dll") + (DWORD)0x2D9D1;
			//DWORD dxgi_base = (DWORD)vtable[8];


			oPresent = ( tPresent )DetourCreate( (PBYTE)dxgi_base, (PBYTE)&hkPresent, 5);



			logFile.Write("Detoured Swapchain Function: %X -> %X ... ", dxgi_base, (DWORD)oPresent);
		}else
		{
			logFile.Write("Detour failed. Swapchain Invalid. %X ... ", g_dxRenderer->pSwapChain);
		}
	}


	return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpvReserved)
{
    if(dwReason == DLL_PROCESS_ATTACH) { 
		CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)CreateDetours,0,0,0);
    } 
    return TRUE;
}