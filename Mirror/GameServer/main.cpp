#include "DxLib.h"
#include "smart_ptr.h"
#include "GlobalData.h"
#include "Direction.h"
#include "Log.h"
#include "Connector.h"
#include "GameMaster.h"

/**********************************************************
*														  *
* setFlag( 1 );�@��initialize�ł��Ȃ���update����܂���� *
*														  *
***********************************************************/

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {
	ChangeWindowMode( 1 );
	SetDoubleStartValidFlag( TRUE );
	SetAlwaysRunFlag( TRUE );
	SetConnectTimeOutWait( 60 );
	SetGraphMode( WIDTH, HEIGHT, 32 );
	if ( DxLib_Init( ) != 0 ) {
		return -1;
	}
	SetDrawScreen( DX_SCREEN_BACK );

	{
		GlobalDataPtr data( new GlobalData( ) );
		DirectionPtr direction( new Direction( SERVER, data ) );

		LogPtr log( new Log( data ) );
		ConnectorPtr connector( new Connector( data, log ) );
		GameMasterPtr master( new GameMaster( data, connector, log ) );

		direction->add( CONNECT, connector );
		direction->add( CONNECT, master );
		data->setScene( CONNECT );

		direction->initialize( );
		// GlobalData �̃t���O�� 0 �ł���ΑS�v���Z�X�I��
		while ( data->getFlag( ) ) {
			if ( ScreenFlip( ) != 0 || ProcessMessage( ) != 0 || ClearDrawScreen( ) != 0 ) {
				break;
			}

			direction->run( );
		}
	}

	DxLib_End( );
	return 0;
}