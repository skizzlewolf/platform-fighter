#pragma once
#include "GameTypes.h"

// ============================================================
//  PLAYER
// ============================================================

struct Player {
	// ----------------------------
	// Constants
	// ----------------------------
	float width = 40.f;
	float height = 60.f;

	// ----------------------------
	// Render + physics
	// ----------------------------
	sf::RectangleShape body;
	sf::Vector2f velocity{ 0.f, 0.f };

	// ----------------------------
	// Movement
	// ----------------------------
	float walkSpeed = 300.f;
	float runSpeed = 520.f;
	float gravity = 2000.f;
	float jumpForce = -700.f;

	float jumpHoldGravityMultiplier = 0.55f;
	float jumpReleaseGravityMultiplier = 1.35f;
	float maxJumpHoldTime = 0.14f;
	float jumpHoldTimer = 0.f;

	float groundFriction = 2600.f;
	float runGroundFriction = 2200.f;

	float airAccel = 1800.f;
	float airDecel = 1400.f;
	float airMaxSpeed = 260.f;
	

	bool fastFalling = false;
	float fastFallSpeed = 650.f; //minimum downward speed when fast-fall starts
	float fastFallGravityBonus = 1600.f;
	

	float landingLagTimer = 0.f;
	float landingLagLight = 0.06f;
	float landingLagHeavy = 0.12f;

	//Jump system(double jumps + multple jumps later)
	int maxJumps = 2; //ground jump + midair jump
	int jumpsRemaining = 2; //refilled on landing
	bool jumpWasDown = false;

	bool isRunning = false;
	float runHoldTimer = 0.f;
	float runHoldThreshold = 0.22f; //hold direction for this long to run
	int runDir = 0; // -1 : left, +1 right : +1

	float runStopGraceMax = 0.08f; // how long you're allowed to have no direction button pressed without losing run
	float runStopGraceTimer = 0.f;
	float runActiveTimer = 0.f;
	float runActiveMin = 0.10f; //min run time to slide

	//Double-tap detection
	float tapWindow = 0.28f; //max time between taps to count as a double tap
	float leftTapTimer = 0.f; //counts down after first left tap
	float rightTapTimer = 0.f; //counts down after first right tap

	bool leftWasDownMove = false; // separate from attack edge detection
	bool rightWasDownMove = false; 

	// ----------------------------
	// Platforming state
	// ----------------------------
	bool onGround = false;
	bool dropping = false;
	float dropTimer = 0.f;
	float dropDuration = 0.2f;

	// ----------------------------
	// Respawn + Stock system
	// ----------------------------
	bool alive = true;
	float respawnTimer = 0.f;
	float respawnDelay = 1.25f;

	float spawnInvulnTimer = 0.f;
	float spawnInvulnDuration = 3.0f;

	// ----------------------------
	// Combat + hitstun
	// ----------------------------
	bool facingRight = true;
	float hitstunTimer = 0.f; // when > 0, player is stunned
	float knockdownTimer = 0.f;
	float knockdownLockMin = 0.18f; // extra security so can't move when in knockdown
	float knockdownLockTimer = 0.f; //counts down after knockdown finishes
	bool wasKnockedDown = false;
	AttackState attack;

	// Autocombo state
	int chainIndex = -1;       // -1 means not in chain
	float chainTimer = 0.f;    // time left to press light again to continue
	float chainWindow = 0.60f; // how long autocombo allows
	bool lastAttackHit = false;

	// Edge-detection (one-press triggers)
	bool lightWasDown = false;
	bool heavyWasDown = false;

	// Hitstop
	float hitstopTimer = 0.f; // freeze time remaining
	float hitstopMax = 0.06f; // default hitstop on hit (will tweak per move later)

	// Input buffering
	float heavyBufferTimer = 0.f;
	float lightBufferTimer = 0.f;
	float inputBufferWindow = 0.18f; // tweak later*********
	AttackID queuedAttack = AttackID::None;

	//Damage Percent Tracking
	float damagePercent = 0.0f;
	float maxDamagePercent = 999.0f;

	//Combat tracking | hitstun scaling
	int comboCount = 0;
	float comboTimer = 0.f;
	float comboResetTime = 0.85f; //if you don't connect within this time, combo drops

	//---------------------------
	//Defense: Block & Dodge
	//---------------------------
	bool defenseWasDown = false;
	bool isBlocking = false;
	bool isDodging = false;
	int dodgeDir = 0; // left: -1 || right: +1
	float dodgeTimer = 0.f;
	float dodgeDuration = 0.11f;
	float dodgeInvulnTimer = 0.f;
	float dodgeInvulnDuration = 0.09f;
	float dodgeSpeed = 980.f;
	float dodgeCooldownTimer = 0.f;
	float dodgeCooldown = 2.15f; // time before you can dodge again
	float blockstunTimer = 0.f;

	// Shield
	float shield = 100.f;
	float maxShield = 100.f;
	float shieldRegenDelayTimer = 0.f;
	float shieldRegenDelay = 1.25f;
	float shieldRegenRate = 22.f; //per second
	bool shieldBroken = false;
	float shieldBreakTimer = 0.f;
	float shieldBreakDuration = 1.25f;

	// Move data
	MoveData lightChain[3];
	MoveData heavyMove;
	MoveData runLightMove;

	// Spawn
	sf::Vector2f spawnPoint{ 0.f, 0.f };

	// Controls
	Controls controls;

	// ----------------------------
	// Constructor (move setup)
	// ----------------------------
	Player(); 

	// ----------------------------
	// Quick checks + data access
	// ----------------------------
	bool isAttacking() const;
	const MoveData* currentMoveData() const;
	bool isLightAttack(AttackID id) const;

	// ----------------------------
	// Attack phase helpers
	// ----------------------------
	float attackElapsed() const;
	bool moveIsInActiveFrames() const;
	void resetCombo();
	void registerComboHit();

	//***************CURRENTLY NOT IN USE*******************
	void applyAttackLunge();

	// Cancel window rule (beginner version)
	// allow Heavy cancel from Light1/Light2/Light3
	// ONLY within cancel window, ONLY if your light actually hit
	bool canCancelToHeavy() const;


	//allow smooth combo from light 2 -> light 3
	bool canChainToNextLight() const;

	//-------------------------
	// Combat locks
	//-------------------------
	bool inKnockdown() const;
	bool movementLocked() const;
	bool actionsLocked() const;
	bool canSpendBufferedInputs() const;

	//-----------------------------
	// State reset helpers
	//-----------------------------
	void stopRun();
	void stopDodge();

	// ----------------------------
	// Hitboxes
	// ----------------------------
	sf::FloatRect makeHitboxRect(const MoveData& m) const;
	sf::FloatRect currentHitbox() const;

	// Phantom Hitbox
	enum class AttackPhase { None, Startup, Active, Recovery };

	AttackPhase attackPhase() const;

	// Planned hitbox
	sf::FloatRect plannedHitbox() const; 

	// ----------------------------
	// Misc accessors
	// ----------------------------
	sf::Vector2f center() const;

	//----------------------------
	// Damage Percentages
	//----------------------------
	void addDamage(float amount);
	void resetDamage();
	float getDamage() const;

	//---------------------------
	// Shield Helper
	//---------------------------
	bool canBlock() const;
	void resetShield();

	// ----------------------------
	// Respawn / life cycle
	// ----------------------------
	void setSpawnPoint(sf::Vector2f p);
	void resetForStageStart(sf::Vector2f spawnPos);
	bool isInvulnerable() const;
	bool canBeHit() const;
	void kill();
	void respawnNow();
	// ----------------------------
	// Hitstop / hitstun / knockdown
	// ----------------------------
	bool inHitstun() const;

	void startHitstop(float seconds);

	// Combat
	sf::FloatRect hurtbox() const;

	void takeHit(
		const sf::Vector2f& baseKnockback,
		float hitstunSeconds,
		float damage,
		float knockbackScaling = 1.0f,
		bool knockdown = false,
		float knockdownSeconds = 0.f);

		

	// ----------------------------
	// Attack start helpers
	// ----------------------------
	void startAttack(AttackID id);
	void startNextLightChain();

	// ----------------------------
	// Update (timers, input, physics, collisions)
	// ----------------------------
	void update(float dt, const std::vector<Platform>& platform);
		
	// ----------------------------
	// Rendering
	// ----------------------------
	void draw(sf::RenderWindow& window, bool showStateTints = true) const;
};