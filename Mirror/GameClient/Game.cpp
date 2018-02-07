#include "Game.h"
#include "GlobalData.h"
#include "Client.h"
#include "Debug.h"
#include "Lazer.h"
#include "Drawer.h"
#include "Image.h"

const int CUTIN_SPEED = 20;
const int WAIT_TIME = 90;

Game::Game( GlobalDataPtr data ) :
_data( data ) {
	setFlag( 1 );
	_field = _data->getFieldPtr( );
	_client = _data->getClientPtr( );
	_drawer = _data->getDrawerPtr( );
	_lazer = LazerPtr( new Lazer( _data ) );
}

Game::~Game( ) {
}

std::string Game::getTag( ) {
	return "GAME";
}

void Game::initialize( ) {
	_turn = 1;
	_player_selected = false;
	_mirror_phase_recv = false;
	_attack_phase_recv = false;
	_judge_phase_recv = false;
	_turn_finish = false;
	_send_live = false;
	_win = false;
	_phase_cutin = false;
	_player_cutin = false;
	_player_num = 0;
	_phase = SET_PLAYER_PHASE;
	_tmp_mirror = Field::Mirror( );
	_order = -1;

	std::array< ImageProperty, CUTIN_MAX >( ).swap( _phase_cutin_image );
	setPhaseCutin( );
	setPlayerCutin( );
}

void Game::update( ) {
	if ( _client->getPhase( ) != "CONNECTING" ) {
		return;
	}

	DebugPtr debug = _data->getDebugPtr( );
	if ( debug->getFlag( ) ) {
		debug->addLog( "Phase      : " + std::to_string( ( int )_phase ) );
		debug->addLog( "Player Num : " + std::to_string( _player_num ) );
		debug->addLog( "Order Num  : " + std::to_string( _client->getOrder( ) ) );
	}

	BATTLE_PHASE phase = _client->getBattlePhase( );
	if ( _phase != phase ) {
		_phase = phase;
		_phase_cutin = false;
		_turn_finish = false;
	}

	if ( _field->getPhase( ) != _phase ) {
 		_field->setPhase( _phase );
	}

	_player_num = _client->getPlayerNum( );
	if ( _player_num != ( unsigned char )-1 ) {
		_field->setPlayerNum( _player_num );
	}

	if ( !_phase_cutin ) {
		calcPhaseCutin( );
		drawPhaseCutin( );
		return;
	}

	switch ( _phase ) {
	case SET_PLAYER_PHASE:
		updatePlayerPhase( );
		break;
	case SET_MIRROR_PHASE: 
		recvMirrorPhase( );
		if ( _mirror_phase_recv ) {
			updateMirrorPhase( );
		}
		break;
	case ATTACK_PHASE:
		recvAttackPhase( );
		if ( _attack_phase_recv ) {
			updateAttackPhase( );
		}
		break;
	case JUDGE_PHASE:
		if ( _turn_finish ) {
			break;
		}
		recvJudgePhase( );
		if ( _judge_phase_recv ) {
			updateJudgePhase( );
		}
		break;
	}
}

void Game::calcPhaseCutin( ) {
	if ( _phase > ATTACK_PHASE ) {
		_phase_cutin = true;
		return;
	}

	int idx = ( int )_phase;

	if ( _phase_cutin_image[ idx ].cx < WIDTH / 2 ) {
		_phase_cutin_image[ idx ].cx += CUTIN_SPEED;
	} else {
		if ( _phase_cutin_image[ idx ].cnt < WAIT_TIME ) {
			_phase_cutin_image[ idx ].cnt++;
			return;
		}
		_phase_cutin_image[ idx ].cx += CUTIN_SPEED;

		if ( _phase_cutin_image[ idx ].cx > WIDTH + _phase_cutin_image[ idx ].lx ) {
			_phase_cutin = true;
		}
	}
}

void Game::calcPlayerCutin( ) {
	int idx = ( _player_num == _order );

	if ( _player_cutin_image[ idx ].cx < WIDTH / 2 ) {
		_player_cutin_image[ idx ].cx += CUTIN_SPEED * 1.5;
		_background_cutin_image.cx += CUTIN_SPEED * 1.5;
	} else {
		if ( _player_cutin_image[ idx ].cnt < WAIT_TIME ) {
			_player_cutin_image[ idx ].cnt++;
			_background_cutin_image.cnt++;
			return;
		}
		_player_cutin_image[ idx ].cx += CUTIN_SPEED;
		_background_cutin_image.cx += CUTIN_SPEED;

		if ( _player_cutin_image[ idx ].cx > WIDTH + _player_cutin_image[ idx ].lx ) {
			_player_cutin = true;
		}
	}
}

void Game::drawPhaseCutin( ) const {
	if ( _phase_cutin ) {
		return;
	}
	_drawer->setImage( _phase_cutin_image[ ( int )_phase ] );
}

void Game::drawPlayerCutin( ) const {
	if ( _player_cutin ) {
		return;
	}
	_drawer->setImage( _background_cutin_image );
	_drawer->setImage( _player_cutin_image[ ( _player_num == _order ) ] );
}

void Game::setPhaseCutin( ) {
	ImagePtr image_ptr = _data->getImagePtr( );
	{//�t�F�[�Y�J�b�g�C���̕���
		for ( int i = 0; i < CUTIN_MAX; i++ ) {
			Png image = image_ptr->getPng( CUTIN_STRING_IMAGE, i );
			_phase_cutin_image[ i ].cx = image.width / 2 * -1;
			_phase_cutin_image[ i ].cy = HEIGHT / 2;
			_phase_cutin_image[ i ].lx = image.width / 2;
			_phase_cutin_image[ i ].ly = image.height / 2;
			_phase_cutin_image[ i ].png = image.png;
			_phase_cutin_image[ i ].cnt = 0;
		}
	}
}

void Game::setPlayerCutin( ) {
	ImagePtr image_ptr = _data->getImagePtr( );
	{//�w�i�C���[�W
		Png image = image_ptr->getPng( CUTIN_IMAGE, 0 );
		_background_cutin_image.cx = image.width / 2 * -1;
		_background_cutin_image.cy = HEIGHT / 2;
		_background_cutin_image.lx = image.width / 2;
		_background_cutin_image.ly = image.height / 2;
		_background_cutin_image.png = image.png;
		_background_cutin_image.cnt = 0;
	}

	{//�v���C���[�J�b�g�C��
		for ( int i = 0; i < PLAYER_NUM; i++ ) {
			Png image = image_ptr->getPng( CUTIN_STRING_IMAGE, CUTIN_MAX + i );
			_player_cutin_image[ i ].cx = image.width / 2 * -1;
			_player_cutin_image[ i ].cy = HEIGHT / 2;
			_player_cutin_image[ i ].lx = image.width / 2;
			_player_cutin_image[ i ].ly = image.height / 2;
			_player_cutin_image[ i ].png = image.png;
			_player_cutin_image[ i ].cnt = 0;
		}
	}
}

void Game::selectPlayerPos( ) {
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
	_field->setTmpPlayerPoint( );
	_player_selected = true;
}

void Game::updatePlayerPhase( ) {
	_field->setInfoText( "���Ȃ��̔z�u�����Ă�������" );
	selectPlayerPos( );

	if ( !_player_selected ) {
		return;
	}

	if ( !_field->isHitDecisionButton( ) ) {
		return;
	}

	if ( !_data->getClickLeft( ) ) {
		return;
	}

	if ( _field->isSelectedPlayer( ) ) {
		return;
	}

	_field->playerPosSelected( );
	_field->setPlayerPoint( _player_num, _field->getTmpPlayerPoint( ) );
	_client->setPlayerPos( _field->getPlayerPoint( _player_num ) );
	_client->sendTcp( );

	setPhaseCutin( );
}

void Game::inputTmpMirror( ) {

	int order = _client->getOrder( );

	if ( _order != order ) {
		_order = order;
		_player_cutin = false;
		setPlayerCutin( );
	}

	_field->setInfoText( "����z�u���Ă��������B" );
	_field->setInfoText( "������x���N���b�N�Ō�����ς����܂�" );

	//�ǂ������ݒu���Ă��邩�̃J�b�g�C��
	if ( !_player_cutin ) {
		calcPlayerCutin( );
		drawPlayerCutin( );
		return;
	}

	if ( _order != _player_num ) {
		return;
	}

	bool hit = false;
	hit = _field->isHitFieldPos( );

	if ( !hit ) {
		return;
	}

	int pos = _field->getFieldPosHitNum( );
	if ( pos < 0 ) {
		return;
	}

	if ( !_data->getClickLeft( ) ) {
		return;
	}
	
	int x = pos % FIELD_COL;
	int y = pos / FIELD_COL;
	if ( _tmp_mirror.x == x && _tmp_mirror.y == y ) {
		_tmp_mirror.angle = ( MIRROR_ANGLE )( ( int )( _tmp_mirror.angle + 1 ) % ( int )MIRROR_ANGLE_MAX );
	} else {
		_tmp_mirror.angle = RIGHT;
	}

	_tmp_mirror.x = x;
	_tmp_mirror.y = y;
	_tmp_mirror.flag = true;

	_field->setTmpMirrorPoint( _player_num, _tmp_mirror.x, _tmp_mirror.y, _tmp_mirror.angle );
}

void Game::updateMirrorPhase( ) {

	inputTmpMirror( );

	if ( !_tmp_mirror.flag ) {
		return;
	}

	bool hit = false;
	hit = _field->isHitDecisionButton( );

	if ( !hit ) {
		return;
	}

	if ( !_data->getClickLeft( ) ) {
		return;
	}

	_field->mirrorPosSelected( );

	_client->setCtsPlayerNum( );
	_client->setCtsAngle( _tmp_mirror.angle );
	_client->setCtsX( _tmp_mirror.x );
	_client->setCtsY( _tmp_mirror.y );
	_client->setCtsFlag( true );

	_client->sendTcp( );

	_tmp_mirror = Field::Mirror( );
	_lazer->initialize( );
	setPhaseCutin( );
	setPlayerCutin( );
	_player_cutin = false;
}

void Game::updateAttackPhase( ) {
	_lazer->update( );
	if ( !_lazer->isFinish( ) ) {
		return;
	}
	if ( _send_live ) {
		return;
	}

	//�����Ƀv���C���[�̐�������
	int dead = _field->getDeadPlayer( );
	bool alive = true;
	if ( dead == _player_num ) {
		alive = !alive;
	}

	_client->setAlive( alive );
	_client->sendTcp( );
	_send_live = true;
	setPhaseCutin( );
}

void Game::updateJudgePhase( ) {
	int winner = _client->getWinner( );

	if ( winner == _player_num ) {
		//����
		_data->setScene( RESULT );
		_win = true;
	} else if ( winner == ( unsigned char )-1 ) {
		//���s�Ȃ�
		_field->mirrorPosSelected( );
		_mirror_phase_recv = false;
		_attack_phase_recv = false;
		_judge_phase_recv = false;
		_send_live = false;
		_tmp_mirror = Field::Mirror( );
		_field->nextTurn( );
		_turn++;
		_field->setTurn( _turn );
		_lazer->clearLazerImage( );
	} else {
		//����
		_data->setScene( RESULT );
		_win = false;
	}

	_turn_finish = true;
	_client->sendTcp( );
}

void Game::recvMirrorPhase( ) {
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

	_mirror_phase_recv = true;
}

void Game::recvAttackPhase( ) {
	if ( !_client->isRecvingUdp( ) ) {
		return;
	}

	if ( _attack_phase_recv ) {
		return;
	}

	std::array< Field::Mirror, PLAYER_NUM > mirror;
	for ( int i = 0; i < PLAYER_NUM; i++ ) {
		mirror[ i ].player_num = _client->getStcPlayerNum( i );
		mirror[ i ].x = _client->getStcX( i );
		mirror[ i ].y = _client->getStcY( i );
		mirror[ i ].angle = _client->getStcAngle( i );
	}

	if ( mirror[ 0 ].x == mirror[ 1 ].x &&
		 mirror[ 0 ].y == mirror[ 1 ].y &&
		 mirror[ 0 ].angle == mirror[ 1 ].angle ) {
		_field->deleteMirrorPoint( mirror[ 0 ].x + mirror[ 0 ].y * FIELD_COL );
	} else {
		_field->setMirrorPoint( mirror[ 0 ].player_num, mirror[ 0 ].x, mirror[ 0 ].y, mirror[ 0 ].angle );
		_field->setMirrorPoint( mirror[ 1 ].player_num, mirror[ 1 ].x, mirror[ 1 ].y, mirror[ 1 ].angle );
	}

	_attack_phase_recv = true;
}

void Game::recvJudgePhase( ) {
	if ( !_client->isRecvingUdp( ) ) {
		return;
	}
	_judge_phase_recv = true;
}

bool Game::isWin( ) const {
	return _win;
}