// FX Library

#include "cg_local.h"
#include "FX_Public.h"

//Emitters
//TODO: FXE_Splash		-	directed splash (conical)
//TODO: FXE_Explosion	-	undirected explosion of particles from a position

void FXE_Spray	( vec3_t direction, float speed, float variation, float cone, float gravity, FXPrimitive *effect )
{
	vec3_t	velocity, acceleration, dir;

	//Randomize the direction
	for ( int i = 0; i < 3; i ++ )
		dir[i] = direction[i] + (cone * crandom());

	VectorNormalize(dir);

	//Get the speed
	VectorScale( dir, speed + (variation * crandom()), velocity );
	effect->SetVelocity( velocity );

	//TODO: Make this more interesting if the user desires
	VectorSet( acceleration, 0, 0, -gravity );
	effect->SetAcceleration( acceleration );
}

