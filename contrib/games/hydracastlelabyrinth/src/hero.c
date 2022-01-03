#include "hero.h"
#include "game.h"
#include <math.h>
#include "weapon.h"
#include "platform.h"
#include <stdio.h>

//State constants
const char NORMAL = 0;
const char SLASH = 1;
const char HIT = 2;
const char LADDER = 3;
const char STONE = 4;
const char CHARGE = 5;
//#define GETITEM 6
const char DOOR = 7;
const char DEATH = 8;
const char QUAKE = 9;

int state;

double herox, heroy;
double herohp, maxhp;
int heroAmmo, maxAmmo;
int heroWeapon;

Mask heroMask;
Mask shieldMask;

void updateMask();
void heroChangeScreen(int dx, int dy);

int herodir = 1;

int canCharge = 0;
int canJump = 0;
int onground = 0;
int heldUp = 0;

const double GRAVITY = 0.3;
const double CLIMBSPEED = 1.2;
const double CLIMBSPEEDPOWER = 2.0;

double vsp = 0;
double hsp = 0;
double imageIndex = 0;
double jumpspd = 7.5;


int invincible = 0;
int timer = 0;
int chargeTimer = 0;
int shieldTimer = 0; //Holds up shield if this is 0
int stun = 0;
int stunTimer = 0;

int poisoned = 0;
int stoneTimer = 0;
int stoneState = 0;
int stoneDir = 1;

int inWater = 0;
int drownTimer = 0;

void heroSetup()
{	
	state = NORMAL;
	herodir = 1;
	
	herox = 320;
	heroy = 320;
	vsp = 0;
	hsp = 0;
	imageIndex = 0;
	//climbspd = 1;
	
	invincible = 0;
	timer = 0;
	chargeTimer = 0;
	shieldTimer = 0;
	
	poisoned = 0;
	stoneTimer = 0;
	stoneState = 0;
	stoneDir = 1;
	
	herohp = 128;
	maxhp = 128;
	heroAmmo = 0;
	maxAmmo = 99;
	heroWeapon = -1;
	
	heroMask.unused = 0;
	heroMask.circle = 0;
	heroMask.w = 24;
	heroMask.h = 26;
	
	onground = 0;
	canJump = 0;
	
	heroy += 1;
	if (checkTileCollision(1, getHeroMask()) == 1 || checkTileCollision(3, getHeroMask()) == 1) {
		onground = 1;
		if (hasItem[12] == 1) {
			canJump = 1;
		}
	}
	heroy -= 1;
	
	shieldMask.unused = 1;
	shieldMask.circle = 0;
	shieldMask.w = 24;
	shieldMask.h = 24;
	shieldMask.x = 0;
	shieldMask.y = 0;
	
	inWater = -1;
}

int heroStep()
{	
	int result = -1;

	//set HP limits
	{
		if (herohp > maxhp) {
			herohp = maxhp;
		}
		
		if (herohp < 0) {
			herohp = 0;
		}
	}

	heldUp = btnUp.held;

	//Counters
	{
		if (invincible > 0) {
			invincible -= 1;
		}
	}
	
	//Scripted states
	if (state == DOOR) {
		//Remove some status conditions
		stun = 0;
		poisoned = 0;
		inWater = 0;
		
		//Animate
		imageIndex += 0.2;
		
		//Done walking
		if (imageIndex >= 10) {
			enterDoor();
			state = NORMAL;
		}
	}
	
	else if (state == DEATH) {
		stun = 0;
		stunTimer = 0;
		poisoned = 0;
			
		//Animate
		{
			imageIndex += 0.3;
			if (imageIndex >= 4) {
				imageIndex -= 4;
			}
			
			//blinking
			if (timer >= 90) {
				invincible = 1;
			}else{
				invincible = 0;
			}
		}
		
		timer += 1;
		
		//Poof
		if (timer == 90) {				
			createEffect(2, herox - 32, heroy - 12);
		}
		//Play Music
		if (timer == 150) {
			PHL_PlayMusic(bgmGameover);
		}
		
		//End game over screen prematurly
		if (timer > 150 && btnStart.pressed == 1) {
			btnStart.pressed = 0;
			timer = 630;
		}
		
		//Reset game
		if (timer == 630) {	
			/*
			FILE* f;
			if ((f = fopen("data/save.tmp", "rb"))) {					
				remove("data/save.tmp");
			}
			fclose(f);
			*/
			PHL_StopMusic();
			result = 1;
		}	
	}
	
	//Uncontrollable states, but can move
	else {
		char canGrav = 1;
		double grav = GRAVITY;
		
		if (state == CHARGE) {
			canGrav = 0;			
			shieldTimer = 10;
			vsp = 0;
			
			//Charge start (rear back)
			{
				if (timer == 0) {
					imageIndex = 0;
					hsp = -2 * herodir;
				}
			}
			
			//Friction
			{
				double fric = 0.3;
				
				if (hsp < 0) {
					hsp += fric;
					if (hsp >= 0) {
						hsp = 0;
					}
				}else if (hsp > 0) {
					hsp -= fric;
					if (hsp <= 0) {
						hsp = 0;
					}
				}
			}
			
			timer += 1;
			
			//Forward charge start
			{
				if (timer == 15) {
					invincible = 35;
					hsp = 7 * herodir;
					PHL_PlaySound(sounds[sndShot01], CHN_WEAPONS);
				}
			}
			
			//Animation
			{
				if (timer > 15) {
					imageIndex = 1;
				}
				if (timer > 19) {
					imageIndex = 2;
				}
				if (timer > 21) {
					imageIndex = 3;
				}
				if (timer == 59) {
					imageIndex = 4;
				}
			}
			
			//Stop
			if (timer == 30) {
				hsp = 0;
			}
			
			//End state
			if (timer >= 60) {
				state = NORMAL;
			}
			
		}
		
		else if (state == HIT) {
			grav = GRAVITY - 0.05;
			
			//timer
			{
				timer -= 1;
				if (timer < 0) {
					timer = 0;
				}
			}
			
			//Animate
			{
				imageIndex += 0.33;
				if (imageIndex >= 2) {
					imageIndex -= 2;
				}
			}			

			if (onground == 1) {
				hsp = 0;
			}
			
			//End hit state
			{
				if (onground == 1 && vsp == 0 && timer == 0) {
					state = NORMAL;
					invincible = 60;
				}
			}
			
		}
		
		else if (state == STONE) {
			grav = GRAVITY - 0.05;
			
			if (stoneState != 2) {
				stoneTimer -= 1;
				//Setup break free animation
				if (stoneTimer <= 0) {
					stoneTimer = 0;
					stoneState = 2;
					imageIndex = 0;

					createRockSmash(herox, heroy + 20);
					herodir = stoneDir;
				}
			}
			
			//Animate
			imageIndex += 0.16;
			
			//Frozen state flashes
			if (stoneState != 2 && imageIndex >= 2) {
				imageIndex = 0;
			}
			
			//In air
			if (stoneState == 0) {
				if (onground == 0) {
					//hsp = -(herodir * 2);
				}else{
					stoneState = 1;
					createEffect(9, herox, heroy + 20);
					createEffect(9, herox, heroy + 20);
				}
			}
			//On ground
			else if (stoneState == 1) {
				hsp = 0;
				if (btnFaceDown.pressed == 1) {
					stoneTimer -= 30;
					createEffect(9, herox, heroy + 20);
				}
			}
			//Break free animation
			else if (stoneState == 2) {
				imageIndex += 0.16;
				
				if ((int)imageIndex == 3) {
					createEffect(8, herox - 32, heroy - 22);
					imageIndex += 0.5;
				}
				
				if (imageIndex >= 17) {
					state = NORMAL;
					stoneState = 0;
				}
			}
		}
		
		else if (state == QUAKE) {
			grav = GRAVITY - 0.05;
			hsp = 0;
			
			if (onground == 1) {
				PHL_PlaySound(sounds[sndPi02], CHN_HERO);
				if (timer == 0) { 
					vsp = -2 - grav;
					onground = 0;
				}
				else if (timer == 1) {
					vsp = -1 - grav;
					onground = 0;
				}
				else if (timer == 2) {
					vsp = -0.5 - grav;
					onground = 0;
				}
				else if (timer == 3) {
					state = NORMAL;
					vsp = 0;
				}
				timer += 1;			
			}
		}
		
		//Controllable states
		else {
			char canWalk = 1;
	
			if (state == NORMAL) {
				//Timers
				{
					if (shieldTimer > 0) {
						shieldTimer -= 1;
					}
				}
				
				//Change direction with buttons
				{
					if (btnLeft.held == 1) {
						herodir = -1;
					}
					if (btnRight.held == 1) {
						herodir = 1;
					}
				}
			
				//Jumping
				{
					if (btnFaceDown.pressed == 1) {
						if (onground == 1 || canJump == 1) {
							if (onground == 0) {
								canJump = 0;
							}
							vsp = -jumpspd;
							onground = 0;
							PHL_PlaySound(sounds[sndJump01], CHN_HERO);
						}					
					}
					
					//cancel jump
					if (vsp < 0 && btnFaceDown.released == 1) {
						vsp = 0;
					}
				}
				
				//Animate
				{
					if (onground == 1 && hsp != 0) {
						imageIndex += 0.1;
						if (imageIndex >= 2) {
							imageIndex -= 2;
						}
					}
				}
				
				//Charging
				{
					if (canCharge == 1 && btnFaceLeft.held == 1) {
						chargeTimer += 1;
						//Create Effects
						if (chargeTimer >= 10 && chargeTimer < 66 && ((chargeTimer - 10) % 8) == 0) {
							createEffect(6, herox, heroy + 20);
						}
						
						if (chargeTimer == 70) {
							PHL_PlaySound(sounds[sndPower01], CHN_SOUND);
						}
					}
					
					if (canCharge == 1 && chargeTimer >= 70 && btnFaceLeft.released == 1) {
						state = CHARGE;
						timer = 0;
						imageIndex = 0;
						addWeapon(SWORD, herox, heroy);
					}
				}
				
				//Attack
				{
					if (stun == 0) {
						//Slash
						if (btnFaceLeft.pressed == 1) {
							state = SLASH;
							imageIndex = 0;
							PHL_PlaySound(sounds[sndShot01], CHN_WEAPONS);
							addWeapon(SWORD, herox, heroy);
						}

						//Weapon
						if (btnFaceRight.pressed == 1) {
							if (heroWeapon != -1) {
								addWeapon(heroWeapon, (int)herox - 20, (int)heroy);
							}
						}
					}
				}
			
				//Grabbing Ladder
				{
					//Grab ladder
					if (btnUp.held == 1) {
						PHL_Rect collide = getTileCollisionXY(2, herox, heroy + 20);
						if (collide.x != -1) {
							state = LADDER;
							canWalk = 0;
							hsp = 0;
							vsp = 0;
							herox = collide.x + 20;
						}
					}
					
					//Climb down onto ladder
					else if (onground == 1 && btnDown.held == 1) {
						PHL_Rect collide = getTileCollisionXY(3, herox, heroy + 40);
						if (collide.x != -1) {
							state = LADDER;
							canWalk = 0;
							hsp = 0;
							vsp = 0;
							herox = collide.x + 20;
							heroy += 1;
						}
					}
				}
				
			}
			
			else if (state == SLASH) {
				shieldTimer = 10;
				
				//Can move in air, not on the ground
				if (onground == 1) {
					canWalk = 0;
					hsp = 0;
				}

				//Animate
				{
					double imgspd = 0.25;
					
					if (imageIndex < 1) {
						imgspd = 0.25;
					}else if (imageIndex < 2) {
						imgspd = 0.34;
					}else if (imageIndex < 3) {
						imgspd = 0.34;
					}else if (imageIndex < 4) {
						imgspd = 0.125;
					}else if (imageIndex < 5) {
						imgspd = 0.5;
					}

					imageIndex += imgspd;
				}
				
				//Finish slash
				{
					if (imageIndex >= 5) {
						state = NORMAL;
						canCharge = hasItem[17]; //Has red scroll
						chargeTimer = 0;
					}
				}
				
			}
			
			else if (state == LADDER) {
				onground = 0;
				canWalk = 0;
				canGrav = 0;
				hsp = 0;
				vsp = 0;
				
				//Generate final climb speed
				double climbspd = CLIMBSPEED;
				{
					//Has power bracelet
					if (hasItem[4] == 1) {
						climbspd = CLIMBSPEEDPOWER;
					}
					
					//Stun slows climb speed
					if (stun > 0) {
						climbspd /= 2;
					}
				}
				
				//Get up/down axis
				int yaxis = btnDown.held - btnUp.held;
				
				//Animate
				if (yaxis != 0) {
					imageIndex += 0.125;
					
					//Limit imageIndex
					if (imageIndex >= 8) {
						imageIndex -= 8;
					}
				}
				
				//Movement
				heroy += climbspd * yaxis;
				
				//Touch ground
				{
					if (yaxis == 1) {
						PHL_Rect collide = getTileCollision(1, getHeroMask());
						if (collide.x != -1) {
							state = NORMAL;
							heroy = collide.y - 40;
							imageIndex = 0;
						}
					}
				}
				
				//Off of ladder
				{
					if (yaxis != 0) {
						if (checkTileCollision(2, getHeroMask()) == 0 && checkTileCollision(3, getHeroMask()) == 0) {
							state = NORMAL;
							if (btnDown.held == 1) {
								onground = 0;
							}
						}
					}
				}
			}
			
			//Walking
			{
				if (canWalk == 1) {
					int xaxis = btnRight.held - btnLeft.held;				
					hsp = 3 * xaxis;
				}
			}
			
			//Cancel jump
			{
				if (vsp < 0 && btnFaceDown.released == 1) {
					vsp = 0;
				}
			}
			
			//Earthquake
			{
				if (hasItem[11] == 0) { //Does not have amulete
					if (quakeTimer > 0 && onground == 1) {
						state = QUAKE;
						vsp = -3 - grav;
						timer = 0;
						PHL_PlaySound(sounds[sndPi02], CHN_HERO);
					}
				}
			}
			
		}
		
		//Movement
		{
			//Used to prevent glitching out on ladder tops
			int precheckladder = checkTileCollision(3, getHeroMask());
		
			//Horizontal movement
			{
				if (hsp != 0) {
					double finalhsp = hsp;
					//Slow when climbing and stunned
					{
						if ( (inWater == 1 && hasItem[5] == 0) || stun == 1) {
							finalhsp /= 4;
						}
					}
					
					//Speed up movement in water
					{
						if (inWater == 1 && hasItem[5] == 1) { //Has fins
							finalhsp = (finalhsp / 3) * 2;
						}
					}
					
					//Move
					herox += finalhsp;
					
					//Stay within screen during boss fight
					{
						if (bossFlag == 1) {
							if (herox < 10) {
								herox = 10;
							}
							if (herox > 630) {
								herox = 630;
							}
						}
					}
					
					//Collide with wall
					{
						PHL_Rect collide = getTileCollision(1, getHeroMask());					
						if (collide.x == -1 && precheckladder == 0) {
							collide = getTileCollision(3, getHeroMask());
						}
						
						//Did collide
						if (collide.x != -1) {
							if (hsp > 0) {
								herox = collide.x - (heroMask.w / 2);
							}else if (hsp < 0) {
								herox = collide.x + 40 + (heroMask.w / 2);
							}
							
							if (state == STONE) {
								herodir *= -1;
							}
						}
					}
					
					//Check if walked off ledge
					if (vsp >= 0) {
						heroy += 1;

						if ( checkTileCollision(1, getHeroMask()) //Solid ground
							|| (hasItem[13] == 1 && checkTileCollision(5, getHeroMask())) //Has red shoes
							|| (precheckladder == 0 && checkTileCollision(3, getHeroMask())) ) //Ladder tops
						{}else{
							onground = 0;
						}
						
						heroy -= 1;
					}
				}
			}
			
			//Vertical Movement
			{
				//Gravity
				if (canGrav == 1 && onground == 0) {
					int maxVsp = 8;
					
					//Water slows movement
					if (inWater == 1) {
						grav *= 0.5;
						maxVsp *= 0.5;
					}
					
					vsp += grav;
					if (vsp > maxVsp) {
						vsp = maxVsp;
					}
				}
				
				//Vertical Movement
				{
					double tempVsp = vsp;
					char landed = 0;
					
					//Water slows movement
					if (inWater == 1 || stun == 1) {
						tempVsp *= 0.5;
					}
					
					//Movement
					heroy += tempVsp;
					
					//Colliding with floor/ceiling
					{
						PHL_Rect collide = getTileCollision(1, getHeroMask());
						if (collide.x == -1&& precheckladder == 0) {
							collide = getTileCollision(3, getHeroMask());
						}
						if (collide.x == -1 && hasItem[13] == 1) { //has red shoes
							collide = getTileCollision(5, getHeroMask());
						}
			
						if (collide.x != -1) {
							//Collide with floor
							if (vsp > 0) {
								heroy = collide.y - 40;
								vsp = 0;
					
								onground = 1;
								if (hasItem[12] == 1) { //Has blue boots
									canJump = 1;
								}								
					
								landed = 1;
							}
							
							//Collide with ceiling
							else if (vsp < 0) {
								heroy = collide.y + 40 - (40 - heroMask.h);
							}
						}
						
						else{
							//Jumpthrough/moving platforms
							if (vsp >= 0) {
								int i;
								for (i = 0; i < MAX_PLATFORMS; i++) {
									if (platforms[i] != NULL) {
										int onPlatTop = 0;
										if (herox - (heroMask.w / 2) > platforms[i]->mask.x + platforms[i]->mask.w || herox + (heroMask.w / 2) < platforms[i]->mask.x) {
										}
										else{
											if (platforms[i]->y == heroy + 40 && vsp >= 0) {
												onPlatTop = 1;
											}
										}
							
										if (onPlatTop == 1 || checkCollision(getHeroMask(), platforms[i]->mask) == 1) {
											heroMask.y -= vsp;
											if (onPlatTop == 1 || checkCollision(heroMask, platforms[i]->mask) == 0) {
												heroy = platforms[i]->mask.y - 40;
												if (vsp != 0) {
													landed = 1;
												}
												vsp = 0;
												onground = 1;
												if (hasItem[12] == 1) {
													canJump = 1;
												}
											}
										}
									}
								}
							}
						}
					}
				
					//Land on ground after a hit
					if (landed == 1 && (state == HIT || state == STONE)) {
						timer = 60;
						PHL_PlaySound(sounds[sndHit01], CHN_HERO);
			
						createEffectExtra(3, herox - 30, heroy + 8, -1, 0, 0);
						createEffectExtra(3, herox - 10, heroy + 8, 1, 0, 0);	
					}
				
				}
			
			}
		
		}
		
		//Water stuff
		{
			//Drown/bubble
			if (inWater == 1) {
				drownTimer -= 1;
				if (drownTimer <= 0) {
					drownTimer = 60;
					if (hasItem[6] == 0) {
						herohp -= 4;
					}
					createEffect(12, herox, heroy + 20);
					PHL_PlaySound(sounds[sndPi06], CHN_SOUND);
				}
			}
			
			//Splash
			if (checkTileCollision(4, getHeroMask())) {
				if (inWater == 0) {			
					drownTimer = 60;
					//Splash effect
					createSplash(herox, heroy);
				}
				inWater = 1;
			}else{
				if (checkTileCollision(6, getHeroMask()) == 0) {
					if (inWater == 1) {			
						//Splash effect
						createSplash(herox, heroy);
					}
					inWater = 0;
				}
			}
		}
		
		//Poison
		{
			if (poisoned > 0) {
				poisoned -= 1;
				if (poisoned % 20 == 0) {
					herohp -= 1;
					createEffect(7, herox, heroy);
				}
			}
		}
		
		//Switch weapon
		{
			int axis = btnR.pressed - btnL.pressed;
			
			if (axis != 0) {
				PHL_PlaySound(sounds[sndPi01], CHN_SOUND);
				
				int i;
				for (i = 1; i <= 5; i++) {
					int thisweapon = heroWeapon + (i * axis);
					
					if (thisweapon >= 5) {
						thisweapon -= 5;
					}					
					if (thisweapon < 0) {
						thisweapon += 5;
					}
					
					if (hasWeapon[thisweapon] == 1) {
						heroWeapon = thisweapon;
						i = 6;
					}
				}
			}
		}
		
		//Collide with lava
		{
			heroy -= 20;
			if (checkTileCollision(5, getHeroMask())) {
				herohp = 0;
				setDrawHP(0);
			}
			heroy += 20;
		}
		
		//Collide with spikes
		{
			PHL_Rect spike = getTileCollision(6, getHeroMask());
			if (spike.x != -1) {
				Mask spikeMask;
				spikeMask.circle = spikeMask.unused = 0;
				spikeMask.x = spike.x + 10;
				spikeMask.y = spike.y + 10;
				spikeMask.w = spikeMask.h = 20;
				
				if (checkCollision(spikeMask, getHeroMask())) {
					heroHit(15, spike.x + 20);
				}
			}
		}
		
		//Death
		{
			if (getDrawHP() <= 0) { //Based on the hud's opinion on player's health, apparently
				state = DEATH;
				timer = 0;
				imageIndex = 0;
				PHL_StopMusic();
				PHL_PlaySound(sounds[sndHit02], CHN_HERO);
			}
		}
	}
	
	//Manage charge
	{
		if (state != NORMAL) {
			canCharge = 0;
			chargeTimer = 0;
		}
			
		if (canCharge == 1) {
			if (btnFaceLeft.held == 0 && btnFaceLeft.pressed == 0) {
				canCharge = 0;
			}
		}
	}
	
	//Screen transitions
	{
		if (herox < -20) {
			herox = 620;
			heroChangeScreen(-1, 0);
		}
		else if (herox > 660) {
			herox = 20;
			heroChangeScreen(1, 0);
		}
		else if (state == LADDER && heroy < -40) {
			heroy = 440;
			heroChangeScreen(0, -1);
		}
		else if (heroy > 480) {
			heroy = 0;
			heroChangeScreen(0, 1);
		}
	}
	
	return result;
}

void heroChangeScreen(int dx, int dy)
{
	vsp = 0;
	chargeTimer = 0;
	canCharge = 0;
	if (hasItem[12] == 1) {
		canJump = 1;
	}
	if (state == HIT || state == SLASH || state == CHARGE) {
		state = NORMAL;		
	}
	
	//Force a black screen
	PHL_DrawRect(0, 0, 640, 480, PHL_NewRGB(0, 0, 0));
	PHL_ForceScreenUpdate();
	
	changeScreen(dx, dy);
}


void heroDraw()
{
	int cropX = 0, cropY = 0;
	int drawShield = 0;
	
	if (state == DOOR) {
		cropY = 160;
		cropX = (int)imageIndex * 40;
	}
	
	
	else if (state == GETITEM) {
		int animation[7] = {0, 1, 2, 3, 2, 0, 1};
		cropY = 40;
		cropX = 320 + (animation[(int)imageIndex] * 40);
	}
	
	//Climbing
	else if (state == LADDER) {
		cropX = 80;
		cropY = 80;
		int animation[8] = {0, 1, 2, 1, 0, 3, 4, 3};
		cropX += 40 * animation[(int)floor(imageIndex)];
	}
	
	
	else if (state == NORMAL)
	{
		if (onground == 1) {
			//Walking
			if (hsp != 0) {
				cropX = floor(imageIndex) * 40;
				if (herodir == -1) {
					cropX += 80;
				}			
			}
			
			//Standing
			else{				
				imageIndex = 0;
				cropX = 0;
				cropY = 0;
				if (hasItem[14] == 1 && shieldTimer <= 0) {
					drawShield = 1;
					cropY = 120;
					if (herodir == -1) {
						cropX += 40;
					}
					if (heldUp == 1) {
						cropX += 80;
					}
				}else{
					if (herodir == -1) {
						cropX += 80;
					}
				}
			}
		}else{
			//Jumping/falling
			if (vsp < 0) {
				imageIndex = 0;
			}else{
				imageIndex = 1;
			}
			cropX = 160 + (40 * imageIndex);
			if (herodir == -1) {
				cropX += 80;
			}
		}
	}else if (state == SLASH)
	{
		//Sword Slash
		int animation[5] = {0, 1, 2, 2, 0};
		
		cropY = 40;
		cropX = 40 * animation[(int)floor(imageIndex)];		
		if (herodir == -1) {
			cropX += 120;
		}
	}
	else if (state == CHARGE) {
		int animation[5] = {0, 1, 2, 2, 0};
		cropY = 40;
		cropX = animation[(int)imageIndex] * 40;
		if (herodir == -1) {
			cropX += 120;
		}
	}
	else if (state == HIT) {
		int thisImage = 12;
		
		if (onground == 0) {
			thisImage = 8;
		}
		
		if (state == STONE) {
			thisImage = 28;
		}	
		
		thisImage += (int)imageIndex;
		if (herodir == -1) {
			thisImage += 2;
		}
		
		cropX = 40 * thisImage;

		while (cropX >= 640) {
			cropX -= 640;
			cropY += 40;
		}
	}
	else if (state == STONE) {
		cropY = 40;
		
		if (stoneState == 0 || stoneState == 1) { //In air/on ground
			int thisImage = (int)imageIndex;
			if (stoneDir == -1) {
				thisImage += 2;
			}
			
			cropX = 480 + (thisImage * 40);
		}
		else if (stoneState == 2) { //Break free
			int animation[17] = {0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 1, 0};
			cropX = 320 + (animation[(int)imageIndex] * 40);
		}
	}
	else if (state == DEATH) {
		if (timer >= 130) {
			char tempDark = roomDarkness;
			roomDarkness = 0;
			
			PHL_DrawTextBold("GAME OVER", 248, 240, YELLOW);
			
			roomDarkness = tempDark;
		}else{
			int frame = 0;
			if (herodir == 1) {
				int animation[4] = {0, 3, 6, 9};
				frame = animation[(int)imageIndex];
			}
			if (herodir == -1) {
				int animation[4] = {2, 1, 4, 11};
				frame = animation[(int)imageIndex];
			}
			cropX = frame * 40;
		}
	}
	else if (state == QUAKE) {
		cropY = 80;
		if (herodir == -1) {
			cropX = 40;
		}
	}
	
	if ((state == HIT && invincible % 6 < 3) || invincible % 2 == 0) {
		PHL_DrawSurfacePart(herox - 20, heroy, cropX, cropY, 40, 40, images[imgHero]);
		if (drawShield == 1) {
			int scx = 320; //Shield crop x
			int sdx = herox - 2, sdy = heroy + 10; //Shield draw x/y
			if (herodir == -1) {
				sdx -= 36;
				scx += 40;
			}
			if (heldUp == 1) {
				scx += 80;
				sdy -= 26;
				sdx -= 8 * herodir;
			}
			PHL_DrawSurfacePart(sdx, sdy, scx, 240, 40, 40, images[imgHero]);
		}
	}
	
	//Draw stun effect
	if (stun == 1) {
		int frame = (int)(((300 - stunTimer) % 32) / 4);
		if (frame == 0) {
			PHL_PlaySound(sounds[sndHit05], CHN_SOUND);
		}
		
		int animation[8] = {0, 1, 2, 1, 0, -1, -1, -1};
		
		if (animation[frame] != -1) {
			PHL_DrawSurfacePart(herox - 32, heroy - 12, 384 + (animation[frame] * 64), 64, 64, 64, images[imgMisc32]);
		}
		
		if (stunTimer <= 0) {
			stun = 0;
			PHL_PlaySound(sounds[sndPower01], CHN_SOUND);
		}else{
			stunTimer -= 1;
		}
	}
	
	//PHL_DrawRect(mask.x, mask.y, mask.w, mask.h, PHL_NewRGB(0x00, 0x00, 0xFF));
	updateMask();
	//PHL_DrawMask(shieldMask);
}

void updateMask()
{
	heroMask.x = herox - 12;
	heroMask.y = heroy + 14;
	
	//Update shield mask
	{
		shieldMask.unused = 1;		
		if (hasItem[14] == 1) { //has shield
			if (state == NORMAL && onground == 1 && hsp == 0 && shieldTimer == 0) {
				shieldMask.unused = 0;
				
				//Shield held in front
				if (heldUp == 0) {
					shieldMask.w = 14;
					shieldMask.h = 20;
					shieldMask.x = herox + 10;
					shieldMask.y = heroy + 20;
					if (herodir == -1) {
						shieldMask.x -= 34;
					}
				}
				
				//Shield above head
				else{
					shieldMask.w = 24;
					shieldMask.h = 8;
					shieldMask.x = herox - 2;
					shieldMask.y = heroy - 2;
					if (herodir == -1) {
						shieldMask.x -= 20;
					}
				}
			}
		}
	}
	
}

int heroHit(int damage, int centerx)
{	
	if (state != HIT && state != DEATH && state != DOOR && (invincible <= 0 || (state == STONE && invincible == 60))) {
		if (state != STONE || (state == STONE && stoneState != 2)) {	
			PHL_PlaySound(sounds[sndHit02], CHN_HERO);
			herohp -= damage;
			
			vsp = -4;
			onground = 0;
			
			if (herox - centerx > 0) {
				herodir = -1;
				hsp = herodir * -2;
			}
			
			if (herox - centerx < 0) {
				herodir = 1;
				hsp = herodir * -2;
			}
			
			if (state != STONE) {
				state = HIT;
			}
			
			return 1;
		}
	}
	return 0;
}

void heroPoison()
{
	if (hasItem[8] != 1) { //Does not have poison resistance ring
		if (poisoned <= 0) {
			poisoned = 300;
		}
	}
}

void heroStone()
{
	//if (((state != HIT && state != DEATH && state != DOOR ) || (state == STONE && stoneState != 2)) && invincible <= 0) {
	if (state != HIT && state != DEATH && state != DOOR && (invincible <= 0 || (state == STONE && invincible == 60))) {
		if (state != STONE || (state == STONE && stoneState != 2)) {
			if (hasItem[9] != 1) { //Does not have green ring
				if (state == STONE) {
					herodir = stoneDir;
				}
				setHeroState(STONE);		
			}
		}
	}
}

//Get-ers and set-ers
Mask getHeroMask()
{
	updateMask();
	return heroMask;
}

int getHeroState()
{
	return state;
}

void setHeroState(int s)
{
	state = s;
	
	//Special cases
	if (s == GETITEM) {
		heldUp = 0;
		//timer = 0;
		//subPosition = GETITEM;
	}
	
	if (s == DOOR) {
		imageIndex = 0;
	}
	
	if (s == STONE) {
		if (stoneTimer <= 0) {
			stoneTimer = 350;
		}
		stoneState = 0;
		invincible = 60;
		stoneDir = herodir;
	}
}

int getHeroInvincible()
{
	return invincible;
}

int getHeroDirection()
{
	return herodir;
}

void setHeroDirection(int d)
{
	herodir = d;
}

double getHeroImageIndex()
{
	return imageIndex;
}

void setHeroImageIndex(double index)
{
	imageIndex = index;
}

double getHeroVsp()
{
	return vsp;
}

double getHeroHsp()
{
	return hsp;
}

void setHeroHsp(double newHsp)
{
	hsp = newHsp;
}

void setHeroVsp(double newVsp)
{
	vsp = newVsp;
}

int getHeroOnground()
{
	return onground;
}

void setHeroOnground(int val)
{
	onground = val;
}

void setHeroTimer(int t)
{
	timer = t;
}

int getHeroPoisoned()
{
	return poisoned;
}

void heroStun()
{
	if (hasItem[10] == 0) { //Does not have cloak
		stun = 1;
		if (stunTimer <= 0) {
			stunTimer = 300;
		}
	}
}

void setHeroCanjump(int set)
{
	canJump = set;
}