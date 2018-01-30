#include "Lazer.h"
#include "GlobalData.h"
#include "Drawer.h"
#include "Debug.h"
#include <math.h>

const double LAZER_SPEED = 5;

Lazer::Lazer( GlobalDataPtr data ) :
_data( data ) {
	setFlag( 1 );
	_drawer = _data->getDrawerPtr( );
	_field = _data->getFieldPtr( );
}

Lazer::~Lazer( ) {
}

std::string Lazer::getTag( ) {
	return "LAZER";
}

void Lazer::initialize( ) {
	_fin = false;
	_distance = 1;
	_start = Field::Vector( );
	_dir_vec = Field::Vector( );
	_unit = Field::Vector( );
	_start = _field->getLazerPoint( );
	updateUnitVector( );
}

void Lazer::update( ) {
	if ( _field->getPhase( ) < ATTACK_PHASE ) {
		return;
	}
	if ( _fin ) {
		return;
	}
	if ( _dir_vec.x + _start.x > START_POS_X + SQUARE_SIZE * FIELD_COL + SQUARE_SIZE ||
		 _dir_vec.x + _start.x < START_POS_X - SQUARE_SIZE ||
		 _dir_vec.y + _start.y > START_POS_Y + SQUARE_SIZE * FIELD_ROW + SQUARE_SIZE ||
		 _dir_vec.y + _start.y < START_POS_Y - SQUARE_SIZE ) {
		_fin = true;
		return;
	}

	double x = _unit.x * LAZER_SPEED;
	double y = _unit.y * LAZER_SPEED;
	_dir_vec.x += x;
	_dir_vec.y -= y;

	Field::Vector tmp = { _start.x + _dir_vec.x, _start.y + _dir_vec.y };
	_field->updateLazerVector( tmp );
	updateUnitVector( );

	//�`��
	double draw_x = _start.x + _dir_vec.x;
	double draw_y = _start.y + _dir_vec.y;
	_drawer->setLine( _start.x, _start.y, draw_x, draw_y );
}

bool Lazer::isFinish( ) const {
	return _fin;
}

void Lazer::updateUnitVector( ) {
	Field::Vector unit = _field->getNextDirect( );
	if ( unit.x == _unit.x && unit.y == _unit.y ) {
		return;
	}
	_unit = unit;
	Field::Vector pos = _field->getReflectionPoint( );
	if ( pos.x != 0 && pos.y != 0 ) {
		_start = pos;
	}
	_dir_vec = Field::Vector( );
}