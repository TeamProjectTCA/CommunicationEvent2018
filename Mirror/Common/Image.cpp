#include "Image.h"
#include "DxLib.h"
#include "Debug.h"
#include "smart_ptr.h"

const std::string PATH = "./Resources/image/";

Image::Image( ) {
	initialize( );
}

Image::~Image( ) {
	InitGraph( );
}

void Image::initialize( ) {
	_dir_num = 0;
	inputFileName( PATH );

	for ( int i = 0; i < _dir_num; i++ ) {
		Directory add;
		memset( &add, 0, sizeof( Directory ) );

		//�f�B���N�g���������
		switch ( ( IMAGE )i ) {
		case NONE_IMAGE		: add.name = "none"				; break;
		default	:
			{
				DebugPtr debug( new Debug( NULL ) );
				debug->error( "Image.cpp : _dir_num�̐��l���傫�����܂�" );
			}
		}
		_data.push_back( add );
	}

	FILE *fp;
	if ( fopen_s( &fp, "filelist.txt", "w" ) != 0 ) {
		exit( 0 );
	}
	for ( int i = 0; i < _file.size( ); i++ ) {
		if ( _file[ i ].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
			fprintf_s( fp, "\n[ %s ]\n", _file[ i ].cFileName );
			continue;
		}
		fprintf_s( fp, "%s\n", _file[ i ].cFileName );
	}
	fclose( fp );

	inputImage( );
}

Png Image::getPng( IMAGE item, int num ) const {
	int dir = item;
	Png tmp;
	memset( &tmp, 0, sizeof( Png ) );
	if ( item > IMAGE_DIR_MAX - 1 ) {
		DebugPtr debug( new Debug( NULL ) );
		debug->error( "�񋓌^�̒ǉ��Y��̉\��������܂�" );
	}

	int size = ( int )_data[ dir ].png.size( );
	if ( size < num ) {
		printfDx( "�v�f���I�[�o�[���Ă��܂��B\n" );
		return tmp;
	}
	return _data[ dir ].png[ num ];
}

void Image::check( int png ) const {
	if ( png != -1 ) {
		return;
	} else {
		printfDx( "Error�F�摜�ǂݍ��݃G���[�I�I\n" );
		WaitKey( );
	}
}

void Image::inputImage( ) {
	int size = ( int )_file.size( );
	int dir = 0;
	std::string path = PATH;
	std::string input = "\0";

	for ( int i = 0; i < size; i++ ) {
		if ( _file[ i ].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
			for ( int j = 0; j < _dir_num; j++ ) {
				if ( strcmp( _file[ i ].cFileName, _data[ j ].name.c_str( ) ) == 0 ) {
					path = PATH + _file[ i ].cFileName + "/";
					input = _file[ i ].cFileName;
					dir = j;
					break;
				}
			}
			continue;
		}

		for ( int j = 0; j < _dir_num; j++ ) {
			if ( strcmp( input.c_str( ), _data[ j ].name.c_str( ) ) == 0 ) {
				Png add;
				memset( &add, 0, sizeof( Png ) );
				
				//�摜���C���v�b�g
				add.png = LoadGraph( ( path + _file[ i ].cFileName ).c_str( ) );

				//�摜�T�C�Y���C���v�b�g
				GetGraphSize( add.png, &add.width, &add.height );

				check( add.png );
				_data[ dir ].png.push_back( add );
			}
		}
	}
}

void Image::inputFileName( std::string path ) {
	WIN32_FIND_DATA find;
	HANDLE handle;

	handle = FindFirstFile( ( path + "*" ).c_str( ), &find );

	if ( handle == INVALID_HANDLE_VALUE ) {
		printfDx( "%s �����݂��܂���B", path.c_str( ) );
		FindClose( handle );
		return;
	}

	do {
		if ( ( find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) {
			if ( strcmp( find.cFileName, "." ) != 0 && strcmp( find.cFileName, ".." ) != 0 ) {
				_dir_num++;
				_file.push_back( find );
				inputFileName( ( path + find.cFileName + "/" ) );
			}
		} else {
			std::string file_name = find.cFileName;
			if ( file_name.find( ".db" ) == std::string::npos ) {
				_file.push_back( find );
			}
		}
	} while ( FindNextFile( handle, &find ) );

	FindClose( handle );
}