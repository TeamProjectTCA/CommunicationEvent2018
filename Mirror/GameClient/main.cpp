#include "DxLib.h"
#include "smart_ptr.h"
#include "GlobalData.h"
#include "Direction.h"
#include "Client.h"
#include "Title.h"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {
	ChangeWindowMode( 1 );
	SetGraphMode( WIDTH, HEIGHT, 32 );
	if ( DxLib_Init( ) != 0 ) {
		return -1;
	}
	SetDrawScreen( DX_SCREEN_BACK );

	GlobalDataPtr data( new GlobalData( ) );
	ClientPtr client( new Client( ) );
	data->setPtr( client );
	TitlePtr title( new Title( data ) );

	DirectionPtr direction( new Direction( CLIENT, data ) );
	direction->add( ALL, title );

	// GlobalData �̃t���O�� 0 �ł���ΑS�v���Z�X�I��
	while ( data->getFlag( ) ) {
		if ( ScreenFlip( ) != 0 || ProcessMessage( ) != 0 || ClearDrawScreen( ) != 0 ) {
			break;
		}

		//�v�Z�t�F�C�Y
		direction->run( );

		//�`��

	}


	DxLib_End( );
	return 0;
}