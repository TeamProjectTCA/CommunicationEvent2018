#include "GameMaster.h"
#include "GlobalData.h"
#include "Connector.h"
#include "Server.h"
#include "Log.h"
#include <random>

GameMaster::GameMaster( GlobalDataPtr data, ConnectorPtr connector, LogPtr log ) :
_data( data ),
_connector( connector ),
_log( log ) {
}

GameMaster::~GameMaster( ) {
}

std::string GameMaster::getTag( ) {
	return "GAMEMASTER";
}

void GameMaster::initialize( ) {
	setFlag( 1 );
	_server = _data->getServerPtr( );
	_matching = false;
	_dice = false;
	memset( _client_data, 0, sizeof( NetWorkData ) * PLAYER_NUM );
	_phase = SET_PHASE;
	_server->setBattlePhase( _phase );
}

void GameMaster::update( ) {
	bool matching = _connector->isMatching( );
	if ( _matching != matching ) {
		_matching = matching;
		if ( !_matching ) {
			initialize( );
		}
	}
	if ( !_matching ) {
		return;
	}

	if ( !_dice ) {
		orderPlayer( );
	}

	switch ( _phase ) {
	case SET_PHASE		: setTurn( )	; break;
	case ATTACK_PHASE	: attackTurn( )	; break;
	}

	_server->sendDataUdp( );

	for ( int i = 0; i < MACHINE_MAX; i++ ) {
		if ( _server->isConnecting( i ) ) {
			if ( _server->isRecving( i ) ) {
				//_client_data[ i ].x = _server->getX( i );
				//_client_data[ i ].y = _server->getY( i );
				//_client_data[ i ].angle = _server->getAngle( i );
				//_client_data[ i ].fin = _server->getFinish( i );
			}
		}
	}
}

void GameMaster::orderPlayer( ) {
	std::random_device rd;
	int idx = rd( ) % 2;
	_client_data[ idx ].order = 1;
	idx = ( idx + 1 ) % 2;
	_client_data[ idx ].order = 2;
	_dice = true;
}

void GameMaster::setTurn( ) {
	int idx = -1;
	for ( int i = 0; i < PLAYER_NUM; i++ ) {
		if ( _client_data[ i ].fin ) {
			continue;
		}
		if ( idx != -1 && _client_data[ i ].order - 1 > idx ) {
			continue;
		}
		idx = i;
	}

	_server->setOrder( idx );

	// SET_PHASE �I��
	if ( idx < 0 ) {
		_phase = ATTACK_PHASE;
		_server->setBattlePhase( _phase );
		return;
	}

	////���݂̃I�[�_�[�v���C���[�̏�񂾂��擾
	//if ( _server->isRecving( idx ) ) {
	//	_client_data[ idx ].x = _server->getX( idx );
	//	_client_data[ idx ].y = _server->getY( idx );
	//	_client_data[ idx ].angle = _server->getAngle( idx );
	//	_client_data[ idx ].fin = _server->getFinish( idx );
	//}

	//if ( _client_data[ idx ].fin ) {
	//	_log->add( "Machine[ " + std::to_string( idx ) + " ] is Finish" );
	//}
}

void GameMaster::attackTurn( ) {
	initialize( );
}