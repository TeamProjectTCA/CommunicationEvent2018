#include "Direction.h"
#include "DxLib.h"

Direction::Direction( std::shared_ptr< GlobalData > data, std::shared_ptr< Debug > debug ) :
_data( data ),
_debug( debug ) {
	initialize( );
}

Direction::~Direction( ) {
}

void Direction::initialize( ) {
	_scene = NONE;
	_data->initialize( );
}

void Direction::update( ) {
	_data->update( );

	_scene = _data->getScene( );
	std::map< SCENE, std::shared_ptr< Base > >::iterator ite;
	ite = _exe.begin( );
	for ( ite; ite != _exe.end( ); ite++ ) {
		if ( ite->first == _data->getScene( ) ) {
			ite->second->update( );
		}
		if ( ite->first == ALL ) {
			ite->second->update( );
		}
	}

	//�f�o�b�OON
	if ( _data->getKeyState( KEY_INPUT_SPACE ) == 1 ) {
		_debug->setFlag( ( _debug->getFlag( ) + 1 ) % 2 );
	}

	//�I��
	if ( _data->getKeyState( KEY_INPUT_ESCAPE ) ) {
		_data->setFlag( 0 );
	}
}

void Direction::initNextProcess( ) {
}

void Direction::add( SCENE scene, std::shared_ptr< Base > ptr ) {
	_exe[ scene ] = ptr;
}

void Direction::run( ) {
	// GlobalData �̃t���O�� 0 �ł���ΑS�v���Z�X�I��
	while ( _data->getFlag( ) ) {
		if ( ScreenFlip( ) != 0 || ProcessMessage( ) != 0 || ClearDrawScreen( ) != 0 ) {
			break;
		}

		//�v�Z�t�F�C�Y
		update( );
	}
}
