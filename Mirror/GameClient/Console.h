#include "Base.h"
#include "smart_ptr.h"
#include "Client.h"

PTR( Console );
PTR( GlobalData );
PTR( Client );
PTR( Drawer );

class Console : public Base {
public:
	Console( GlobalDataPtr data );
	virtual ~Console( );

public:
	std::string getTag( );
	void initialize( );
	void update( );

private:
	void ready( );
	void matching( );

private:
	GlobalDataPtr _data;
	ClientPtr _client;
	DrawerPtr _drawer;
};