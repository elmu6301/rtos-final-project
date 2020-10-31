//***********************************************************************************
// Include files
//***********************************************************************************


//***********************************************************************************
// defined files
//***********************************************************************************

//Game Speeds
#define DEFAULT_GAME_SPEED  50
#define MAX_GAME_SPEED 10
#define MIN_GAME_SPEED  100


//***********************************************************************************
// global variables
//***********************************************************************************

//Dino State Enumerations
enum DINO_STATES{
	RUN,
	JUMP,
	DUCK
};

//Structure for storing information on the Dino (Player)
typedef struct{
	int state;
}DINO_STRUCT;

//Structure for storing information on the game
typedef struct{
	int speed;
}GAME_STRUCT;



//***********************************************************************************
// function prototypes
//***********************************************************************************
void game_setup();


