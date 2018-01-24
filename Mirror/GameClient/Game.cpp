#include "Game.h"
#include "GlobalData.h"
#include "Client.h"
#include "Debug.h"
#include "Field.h"

Game::Game( GlobalDataPtr data ) :
_data( data ) {
	_field = _data->getFieldPtr( );
}

Game::~Game( ) {
}

std::string Game::getTag( ) {
	return "GAME";
}

void Game::initialize( ) {
	setFlag( 1 );
	_client = _data->getClientPtr( );
	_player_num = 0;
	_phase = SET_PLAYER_PHASE;
}

void Game::update( ) {
	if ( _client->getPhase( ) != "CONNECTING" ) {
		return;
	}
	_phase = _client->getBattlePhase( );

	if ( _field->getPhase( ) != _phase ) {
 		_field->setPhase( _phase );
	}

	_player_num = _client->getPlayerNum( );
	if ( _player_num != ( unsigned char )-1 ) {
		_field->setPlayerNum( _player_num );
	}

	switch ( _phase ) {
	case SET_PLAYER_PHASE	: updatePlayerPhase( )	; break;
	case SET_MIRROR_PHASE	: updateMirrorPhase( )	; break;
	case ATTACK_PHASE		: updateAttackPhase( )	; break;
	}

	DebugPtr debug = _data->getDebugPtr( );
	if ( debug->getFlag( ) ) {
		debug->addLog( "Phase      : " + std::to_string( ( int )_phase ) );
		debug->addLog( "Player Num : " + std::to_string( _player_num ) );
		debug->addLog( " Order Num : " + std::to_string( _client->getOrder( ) ) );
	}
}

void Game::updatePlayerPhase( ) {
	bool hit = false;
	hit = _field->isHitPlayerPos( );

	if ( !hit ) {
		return;
	}
	int pos = _field->getPlayerPosHitNum( );
	if ( pos < 0 ) {
		return;
	}
	if ( !_data->getClickLeft( ) ) {
		return;
	}
	_field->playerPosSelected( );
	_field->setPlayerPoint( _player_num, pos );

	_client->setPlayerPos( pos );
	_client->sendTcp( );
}

void Game::updateMirrorPhase( ) {
	if ( !_client->isRecvingUdp( ) ) {
		return;
	}

	for ( int i = 0; i < PLAYER_NUM; i++ ) {
		int pos = _client->getPlayerPos( i );
		if ( pos == ( unsigned char )-1 ) {
			return;
		}
		_field->setPlayerPoint( i, pos );
	}

	int lazer_pos = _client->getLazerPoint( );
	if ( lazer_pos == ( unsigned char )-1 ) {
		return;
	}
	_field->setLazerPoint( lazer_pos );

	if ( _client->getOrder( ) != _player_num ) {
		return;
	}
	if ( _data->getKeyState( KEY_INPUT_RETURN ) == 1 ) {
		_client->sendTcp( );
	}
}

void Game::updateAttackPhase( ) {
	initialize( );
	_field->initialize( );
}