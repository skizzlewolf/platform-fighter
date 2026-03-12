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
	float chainWindow = 0.45f; // how long autocombo allows
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
	float dodgeCooldown = 0.65f; // time before you can dodge again

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
	Player() {
		body.setSize({ width, height });
		body.setFillColor(sf::Color::Blue);
		body.setPosition({ -100.f, 100.f });

		// Light 1: quick poke (combo starter)
		lightChain[0] = MoveData{
			.startup = 0.04f, .active = 0.06f, .recovery = 0.16f,
			.hitboxSize = {55.f, 22.f},
			.hitboxOffset = {width, height * 0.35f},
			.knockback = {90.f, -20.f},
			.hitstun = 0.28f,

			.cancelStart = 0.04f + 0.06f,
			.cancelEnd = (0.04f + 0.06f) + 0.16f * HEAVY_CANCEL_RECOVERY_FRACTION,
			.cancelOnHitOnly = true,

			.lungeSpeed = 0.f,
			.lungeDuration = 0.f
		};

		// Light 2: a bit bigger
		lightChain[1] = MoveData{
			.startup = 0.05f, .active = 0.06f, .recovery = 0.16f,
			.hitboxSize = {60.f, 22.f},
			.hitboxOffset = {width, height * 0.33f},
			.knockback = {110.f, -25.f},
			.hitstun = 0.30f,

			.cancelStart = 0.05f + 0.06f, // start of recovery
			.cancelEnd = (0.05f + 0.06f) + 0.16f * HEAVY_CANCEL_RECOVERY_FRACTION,
			.cancelOnHitOnly = true
		};

		// Light 3
		lightChain[2] = MoveData{
			.startup = 0.06f, .active = 0.07f, .recovery = 0.14f,
			.hitboxSize = {65.f, 24.f},
			.hitboxOffset = {width, height * 0.30f},
			.knockback = {140.f, -35.f},
			.hitstun = 0.33f,

			.cancelStart = 0.06f + 0.07f,
			.cancelEnd = (0.06f + 0.07f) + 0.14f * HEAVY_CANCEL_RECOVERY_FRACTION,
			.cancelOnHitOnly = true
		};

		// Heavy: slower, bigger, stronger
		heavyMove = MoveData{
			.startup = 0.10f, .active = 0.08f, .recovery = 0.22f,
			.hitboxSize = {70.f, 28.f},
			.hitboxOffset = {width, height * 0.25f},
			.knockback = {420.f, -220.f},
			.hitstun = 0.35f,

			.lungeSpeed = 260.f,
			.lungeDuration = 0.08f
		};

		//Run Light: slide knockdown starter (no run heavy exists)
		runLightMove = MoveData{
			.startup = 0.06f, .active = 0.08f, .recovery = 0.22f,
			.hitboxSize = {75.f,20.f},
			.hitboxOffset{width, height * 0.72f}, //low hitbox for slide
			.knockback = {260.f, -10.f}, //mostly horizontal
			.hitstun = 0.30f,

			.cancelStart = 0.f,
			.cancelEnd = 0.f,
			.cancelOnHitOnly = true,

			.slideSpeed = 620.f,
			.slideDuration = 0.18f
		};
	}

	// ----------------------------
	// Quick checks + data access
	// ----------------------------
	bool isAttacking() const { return attack.current != AttackID::None; }

	const MoveData* currentMoveData() const {
		switch (attack.current) {
		case AttackID::Light1: return &lightChain[0];
		case AttackID::Light2: return &lightChain[1];
		case AttackID::Light3: return &lightChain[2];
		case AttackID::Heavy:  return &heavyMove;
		case AttackID::RunLight: return &runLightMove;
		default: return nullptr;
		}
	}

	bool isLightAttack(AttackID id) const {
		return id == AttackID::Light1 || id == AttackID::Light2 || id == AttackID::Light3;
	}

	// ----------------------------
	// Attack phase helpers
	// ----------------------------
	float attackElapsed() const {
		const MoveData* m = currentMoveData();
		if (!m) { return 0.f; }
		float total = m->startup + m->active + m->recovery;
		return total - attack.timer; // because timer counts down
	}

	bool moveIsInActiveFrames() const {
		const MoveData* m = currentMoveData();
		if (!m) { return false; }

		// time remaining counts down -> time elapsed
		float total = m->startup + m->active + m->recovery;
		float elapsed = total - attack.timer;

		return elapsed >= m->startup && elapsed < (m->startup + m->active);
	}

	//***************CURRENTLY NOT IN USE*******************
	void applyAttackLunge() {
		//Only applies while alive, not stunned, or currently attacking
		if (!alive) { return; }
		if (inHitstun()) { return; }
		if (!isAttacking()) { return; }

		const MoveData* m = currentMoveData();
		if (!m) { return; }

		if (m->lungeSpeed <= 0.f) { return; }
		if (m->lungeDuration <= 0.f) { return; }

		float e = attackElapsed();
		if (e > m->lungeDuration) { return; }

		//Inject controlled horizontal velocity
		//This will override walking input(already disabled during attacks)
		float dir = facingRight ? 1.f : -1.f;
		velocity.x = dir * m->lungeSpeed;
	}
	//******************************************************

	// Cancel window rule (beginner version)
	// allow Heavy cancel from Light1/Light2/Light3
	// ONLY within cancel window, ONLY if your light actually hit
	bool canCancelToHeavy() const {
		if (!isAttacking()) { return false; }

		const MoveData* m = currentMoveData();
		if (!m) { return false; }

		if (!(attack.current == AttackID::Light1 ||
			attack.current == AttackID::Light2 ||
			attack.current == AttackID::Light3)) {
			return false;
		}

		float e = attackElapsed();
		bool inWindow = (e >= m->cancelStart && e <= m->cancelEnd);

		if (!inWindow) { return false; }
		if (m->cancelOnHitOnly && !attack.hasHit) { return false; }

		return true;
	}

	//-------------------------
	// Combat locks
	//-------------------------
	bool inKnockdown() const { return knockdownTimer > 0.f; }

	bool movementLocked() const {
		//no movement control when stunned or attacking
		return inHitstun() || inKnockdown() || knockdownLockTimer > 0.f || isAttacking();
	}

	bool actionsLocked() const {
		//no starting attacks while stunned, dodging, or blocking
		return inHitstun() || inKnockdown() || knockdownLockTimer > 0.f || isDodging || isBlocking;
	}

	bool canSpendBufferedInputs() const {
		return !actionsLocked();
	}

	//-----------------------------
	// State reset helpers
	//-----------------------------
	void stopRun() {
		isRunning = false;
		runDir = 0;
		runActiveTimer = 0.f;
		runStopGraceTimer = 0.f;
		leftTapTimer = 0.f;
		rightTapTimer = 0.f;
	}

	void stopDodge() {
		isDodging = false;
		dodgeDir = 0;
		dodgeTimer = 0.f;
		dodgeInvulnTimer = 0.f;
	}

	// ----------------------------
	// Hitboxes
	// ----------------------------
	sf::FloatRect makeHitboxRect(const MoveData& m) const {
		if (!alive) return sf::FloatRect({}, {});

		sf::Vector2f pos = body.getPosition();
		sf::Vector2f off = m.hitboxOffset;

		if (attack.variant == AttackVariant::Side) {
			off.x += 12.f;
		}

		float hbW = m.hitboxSize.x;
		float hbH = m.hitboxSize.y;

		float localX = facingRight ? off.x : (width - off.x - hbW);
		float hbX = pos.x + localX;
		float hbY = pos.y + off.y;

		return sf::FloatRect({ hbX, hbY }, { hbW, hbH });
	}

	sf::FloatRect currentHitbox() const {
		const MoveData* m = currentMoveData();
		if (!m) { return sf::FloatRect({}, {}); }
		if (!moveIsInActiveFrames()) { return sf::FloatRect({}, {}); }
		return makeHitboxRect(*m);
	}

	// Phantom Hitbox
	enum class AttackPhase { None, Startup, Active, Recovery };

	AttackPhase attackPhase() const {
		const MoveData* m = currentMoveData();
		if (!m) { return AttackPhase::None; }

		float total = m->startup + m->active + m->recovery;
		float elapsed = total - attack.timer; // timer counts down

		if (elapsed < 0.f) { return AttackPhase::None; }
		if (elapsed < m->startup) { return AttackPhase::Startup; }
		if (elapsed < (m->startup + m->active)) { return AttackPhase::Active; }
		if (elapsed < (m->startup + m->active + m->recovery)) { return AttackPhase::Recovery; }

		return AttackPhase::None;
	}

	// Planned hitbox
	sf::FloatRect plannedHitbox() const {
		const MoveData* m = currentMoveData();
		if (!m) { return sf::FloatRect({}, {}); }
		return makeHitboxRect(*m);
	}

	// ----------------------------
	// Misc accessors
	// ----------------------------
	sf::Vector2f center() const {
		return {
			body.getPosition().x + body.getSize().x * 0.5f,
			body.getPosition().y + body.getSize().y * 0.5f
		};
	}

	// ----------------------------
	// Respawn / life cycle
	// ----------------------------
	void setSpawnPoint(sf::Vector2f p) { spawnPoint = p; }

	void resetForStageStart(sf::Vector2f spawnPos) {
		setSpawnPoint(spawnPos);
		respawnNow();
		dropping = false;
		dropTimer = 0.f;
	}

	bool isInvulnerable() const { return spawnInvulnTimer > 0.f; }
	bool canBeHit() const {
		return alive && !isInvulnerable() && !(dodgeInvulnTimer > 0.f);
	}

	void kill() {
		if (!alive) return; // already dead
		alive = false;
		respawnTimer = respawnDelay;
		velocity = { 0.f, 0.f };
	}

	void respawnNow() {
		body.setPosition(spawnPoint);
		velocity = { 0.f, 0.f };
		alive = true;

		// reset movement state
		onGround = false;
		dropping = false;
		dropTimer = 0.f;

		// start invulnerability
		spawnInvulnTimer = spawnInvulnDuration;
	}

	// ----------------------------
	// Hitstop / hitstun / knockdown
	// ----------------------------
	bool inHitstun() const { return hitstunTimer > 0.f || knockdownTimer > 0.f; }

	void startHitstop(float seconds) { hitstopTimer = seconds; }

	// Combat
	sf::FloatRect hurtbox() const { return body.getGlobalBounds(); }

	void takeHit(const sf::Vector2f& knockback, float hitstunSeconds, bool knockdown = false, float knockdownSeconds = 0.f) {
		if (!canBeHit()) return;

		if (isBlocking) {
			sf::Vector2f reducedKB{ knockback.x * 0.25f, knockback.y * 0.15f };
			float reducedStun = hitstunSeconds * 0.40f;

			// Block prevents knockdown
			knockdown = false;
			knockdownSeconds = 0.f;

			//Apply reduced impact
			hitstunTimer = reducedStun;
			velocity = reducedKB;
			onGround = false;

			return;
		}

		//Getting hit stops dodge
		stopDodge();

		//Getting hit always stops run
		stopRun();

		// cancel current attack
		attack.current = AttackID::None;
		attack.timer = 0.f;
		attack.hasHit = false;

		// reset chain (optional)
		chainIndex = -1;
		chainTimer = 0.f;

		hitstunTimer = hitstunSeconds;
		if (knockdown) { knockdownTimer = knockdownSeconds; }
		if (knockdown) {
			stopRun();
			stopDodge();
			isBlocking = false;

			//kill horizontal carry immediately
			velocity.x = 0.f;
		}

		velocity = knockback;
		onGround = false;
	}

	// ----------------------------
	// Attack start helpers
	// ----------------------------
	void startAttack(AttackID id) {
		if (!alive) return;
		if (inHitstun()) return;
		if (isAttacking()) return;

		attack.current = id;
		attack.hasHit = false;
		lastAttackHit = false;

		//Slide end run state (even if you're still holding direction)
		if (id == AttackID::RunLight) {
			stopRun();
		}

		bool holdLeft = sf::Keyboard::isKeyPressed(controls.left);
		bool holdRight = sf::Keyboard::isKeyPressed(controls.right);

		// side attack if you're holding a direction
		//BUT: only Light1 can be side. Light2/ Light3 should stay neutral
		if (id == AttackID::Light1) {
			attack.variant = (holdLeft || holdRight) ? AttackVariant::Side : AttackVariant::Neutral;
		}
		else if (id == AttackID::Light2 || id == AttackID::Light3) {
			attack.variant = AttackVariant::Neutral;
		}
		else {
			//Heavy (and future moves can still use direction)
			attack.variant = (holdLeft || holdRight) ? AttackVariant::Side : AttackVariant::Neutral;
		}

		const MoveData* m = currentMoveData();
		attack.timer = m ? (m->startup + m->active + m->recovery) : 0.f;
	}

	void startNextLightChain() {
		// if not in chain, start at 0
		if (chainIndex < 0) { chainIndex = 0; }
		else { chainIndex++; }

		if (chainIndex > 2) { // end of chain
			chainIndex = -1;
			return;
		}

		AttackID id =
			(chainIndex == 0) ? AttackID::Light1 :
			(chainIndex == 1) ? AttackID::Light2 :
			AttackID::Light3;

		startAttack(id);
	}

	// ----------------------------
	// Update (timers, input, physics, collisions)
	// ----------------------------
	void update(float dt, const std::vector<Platform>& platforms) {
		// Reset per-frame state
		onGround = false;

		// ----------------------------
		// Timers (always)
		// ----------------------------
		if (spawnInvulnTimer > 0.f) {
			spawnInvulnTimer -= dt;
			if (spawnInvulnTimer < 0.f) spawnInvulnTimer = 0.f;
		}

		if (!alive) {
			respawnTimer -= dt;
			if (respawnTimer <= 0.f) {
				respawnNow();
			}
			return; // dead players don't move or collide
		}

		// hitstop (combo feel)
		if (hitstopTimer > 0.f) {
			hitstopTimer -= dt;
			if (hitstopTimer < 0.f) hitstopTimer = 0.f;
			return; // freeze the whole update while hitstop is active
		}

		// ----------------------------
		// Chain window timer
		// ----------------------------
		if (chainTimer > 0.f) {
			chainTimer -= dt;
			if (chainTimer < 0.f) chainTimer = 0.f;
		}
		else {
			// if you wait too long to attack
			if (!isAttacking()) chainIndex = -1;
		}

		// ----------------------------
		// Input buffer timers
		// ----------------------------
		if (lightBufferTimer > 0.f) {
			lightBufferTimer -= dt;
			if (lightBufferTimer < 0.f) lightBufferTimer = 0.f;
		}

		if (heavyBufferTimer > 0.f) {
			heavyBufferTimer -= dt;
			if (heavyBufferTimer < 0.f) heavyBufferTimer = 0.f;
		}

		// ----------------------------
		// Attack timer + hit-confirm memory
		// ----------------------------
		if (isAttacking()) {
			attack.timer -= dt;
			if (attack.timer <= 0.f) {
				AttackID finished = attack.current; // remember what ended
				bool finishedHit = attack.hasHit;   // capture BEFORE reset

				attack.current = AttackID::None;
				attack.timer = 0.f;
				attack.hasHit = false;

				// record hit-confirm result for chaining/buffering logic
				lastAttackHit = finishedHit;

				// open chain window only if the finished light actually hit
				if (isLightAttack(finished) && finishedHit) {
					chainTimer = chainWindow;
				}
				else {
					chainTimer = 0.f;
					chainIndex = -1;
				}
			}
		}

		//-----------------------------
		// Double-tap Timers
		//-----------------------------
		if (leftTapTimer > 0.f) {
			leftTapTimer -= dt;
			if (leftTapTimer < 0.f) { leftTapTimer = 0.f; }
		}

		if (rightTapTimer > 0.f) {
			rightTapTimer -= dt;
			if (rightTapTimer < 0.f) { rightTapTimer = 0.f; }
		}

		//----------------------------
		// Run Ative Timer
		//----------------------------
		if (runActiveTimer > 0.f) {
			runActiveTimer -= dt;
			if (runActiveTimer < 0.f) { runActiveTimer = 0.f; }
		}

		//--------------------------
		// Dodge Timers
		//--------------------------
		if (dodgeTimer > 0.f) {
			dodgeTimer -= dt;
			if (dodgeTimer < 0.f) { dodgeTimer = 0.f; }
		}

		if (isDodging && dodgeTimer <= 0.f) {
			stopDodge();
		}

		if (dodgeInvulnTimer > 0.f) {
			dodgeInvulnTimer -= dt;
			if (dodgeInvulnTimer < 0.f) {
				dodgeInvulnTimer = 0.f;
			}
		}

		if (dodgeCooldownTimer > 0.f) {
			dodgeCooldownTimer -= dt;
			if (dodgeCooldownTimer < 0.f) { dodgeCooldownTimer = 0.f; }
		}

		// ----------------------------
		// Input edge detection -> buffer intent
		// ----------------------------
		bool lightDown = sf::Keyboard::isKeyPressed(controls.light);
		bool lightPressed = lightDown && !lightWasDown;
		lightWasDown = lightDown;

		bool heavyDown = sf::Keyboard::isKeyPressed(controls.heavy);
		bool heavyPressed = heavyDown && !heavyWasDown;
		heavyWasDown = heavyDown;

		if (lightPressed) { lightBufferTimer = inputBufferWindow; }
		if (heavyPressed) { heavyBufferTimer = inputBufferWindow; }

		// ----------------------------
		// Spend buffered inputs (priority: heavy then light)
		// ----------------------------
		if (canSpendBufferedInputs()) {
			// heavy buffered: try cancel first
			if (heavyBufferTimer > 0.f) {
				if (isAttacking() && canCancelToHeavy()) {
					// cancel current light into heavy
					attack.current = AttackID::None;
					attack.timer = 0.f;
					attack.hasHit = false;

					chainIndex = -1;
					chainTimer = 0.f;

					heavyBufferTimer = 0.f;
					startAttack(AttackID::Heavy);
				}
				else if (!isAttacking()) {
					chainIndex = -1;
					chainTimer = 0.f;

					heavyBufferTimer = 0.f;
					startAttack(AttackID::Heavy);
				}
			}

			// light buffered: start / continue chain if allowed
			if (lightBufferTimer > 0.f) {
				if (!isAttacking()) {
					lightBufferTimer = 0.f;

					bool canRunSlide = (isRunning && runActiveTimer > 0.f);
					if (canRunSlide) {
						//Run Light = slide
						chainIndex = -1;
						chainTimer = 0.f;
						startAttack(AttackID::RunLight);
					}
					else {
						startNextLightChain();
					}
				}
				else if (chainTimer > 0.f && lastAttackHit) {
					lightBufferTimer = 0.f;
					startNextLightChain();
				}
			}
		}

		// ----------------------------
		// Movement input (disabled in hitstun/attacking)
		// ----------------------------
		if (!movementLocked()) {
			velocity.x = 0.f;

			bool leftDown = sf::Keyboard::isKeyPressed(controls.left);
			bool rightDown = sf::Keyboard::isKeyPressed(controls.right);
			bool leftPressed = leftDown && !leftWasDownMove;
			bool rightPressed = rightDown && !rightWasDownMove;
			leftWasDownMove = leftDown;
			rightWasDownMove = rightDown;

			//-------------------------
			// Defense input
			//-------------------------
			bool defenseDown = sf::Keyboard::isKeyPressed(controls.defense);
			bool defensePressed = defenseDown && !defenseWasDown;
			defenseWasDown = defenseDown;

			//Determine sideways intent (block vs dodge)
			int sideDir = 0;
			if (leftDown && !rightDown) { sideDir = -1; }
			else if (rightDown && !leftDown) { sideDir = +1; }

			//if you start moving, block must immediately drop
			if (sideDir != 0) {
				isBlocking = false;
			}

			//----------------------
			// Block (hold)
			//----------------------
			if (!isDodging && defenseDown && sideDir == 0) {
				isBlocking = true;
			}
			else if (!defenseDown) {
				isBlocking = false;
			}

			//----------------------
			// Dodge (press while moving side-to-side)
			//----------------------
			bool wantsDodge = defensePressed;
			if (wantsDodge && sideDir != 0 && !isDodging && dodgeCooldownTimer <= 0.f) {
				isDodging = true;
				isBlocking = false;

				dodgeDir = sideDir;
				dodgeTimer = dodgeDuration;
				dodgeInvulnTimer = dodgeInvulnDuration;
				dodgeCooldownTimer = dodgeCooldown;

				//Dodge cancels run
				stopRun();

				//Clear buffered attacks
				lightBufferTimer = 0.f;
				heavyBufferTimer = 0.f;
				chainTimer = 0.f;
				chainIndex = -1;
			}

			if (isBlocking) {  //******** MAY ADJUST LATER**********
				isRunning = false;
				runDir = 0;
				velocity.x = 0.f;
			}

			//--------------------------
			// Double-tap detection
			//--------------------------
			if (leftPressed) {
				if (leftTapTimer > 0.f) {
					isRunning = true;
					runDir = -1;
					leftTapTimer = 0.f;
					rightTapTimer = 0.f;
				}
				else {
					leftTapTimer = tapWindow;
				}
			}
			if (rightPressed) {
				if (rightTapTimer > 0.f) {
					isRunning = true;
					runDir = +1;
					rightTapTimer = 0.f;
					leftTapTimer = 0.f;
				}
				else {
					rightTapTimer = tapWindow;
				}
			}

			//----------------------------
			// Resolve intended direction
			//----------------------------
			int dir = 0;
			if (leftDown && !rightDown) { dir = -1; }
			else if (rightDown && !leftDown) { dir = +1; }
			else { dir = 0; } //none or both

			//Stop running if you let go or switch direction
			if (dir == 0) {
				//short grace period (direction switches)
				if (isRunning) {
					runStopGraceTimer -= dt;
					if (runStopGraceTimer <= 0.f) {
						isRunning = false;
						runDir = 0;
						runStopGraceTimer = 0.f;
					}
				}
				else {
					runStopGraceTimer = 0.f;
				}
			}
			else {
				//holding a direction, so refresh grace period
				runStopGraceTimer = runStopGraceMax;

				//if already running, changing direction keeps you running
				if(isRunning) {
					runDir = dir;
				}

				float speed = isRunning ? runSpeed : walkSpeed;
				if (isRunning) { runActiveTimer = runActiveMin; }
				velocity.x = dir * speed;
				facingRight = (dir > 0);

			}
		}	

		//-------------------------------
		// Attack lunge injection (Light1 + Heavy only)
		//-------------------------------
	
		//applyAttackLunge();

		//-------------------------------
		// RunLight slide movement (only during early frames)
		//-------------------------------
		if (!inHitstun() && isAttacking() && attack.current == AttackID::RunLight && attack.variant == AttackVariant::Side) {
			const MoveData* m = currentMoveData();
			float e = attackElapsed();
			if (m && e <= m->slideDuration) { 
				float dir = facingRight ? 1.f : -1.f;
				velocity.x = dir * m->slideSpeed; 
			}
		}

		//-----------------------------
		// Stop leftover slide/run velocity during attacks
		//-----------------------------
		if (!inHitstun() && isAttacking()) {
			bool allowSlide =
				(attack.current == AttackID::Light1 && attack.variant == AttackVariant::Side);
			bool allowRunSlide =
				(attack.current == AttackID::RunLight);

			if (!allowSlide && !allowRunSlide) {
				velocity.x = 0.f;
			}
		}

		//-----------------------------
		// Dodge Movement injection
		//-----------------------------
		if (!inHitstun() && isDodging) {
			velocity.x = dodgeDir * dodgeSpeed;
		}

		// ----------------------------
		// Physics
		// ----------------------------
		sf::Vector2f previousPosition = body.getPosition();

		// Gravity
		velocity.y += dt * gravity;

		// Horizontal move + collide (solids)
		body.move({ velocity.x * dt, 0.f });
		sf::FloatRect playerBounds = body.getGlobalBounds();

		for (const auto& platform : platforms) {
			if (platform.oneWay) continue;

			sf::FloatRect platformBounds = platform.shape.getGlobalBounds();
			if (playerBounds.findIntersection(platformBounds)) {

				if (velocity.x > 0.f) {
					body.setPosition({ platformBounds.position.x - playerBounds.size.x, body.getPosition().y });
				}
				else if (velocity.x < 0.f) {
					body.setPosition({ platformBounds.position.x + platformBounds.size.x, body.getPosition().y });
				}

				playerBounds = body.getGlobalBounds();
			}
		}

		// Vertical move + collide (solids + oneWays)
		body.move({ 0.f, velocity.y * dt });
		playerBounds = body.getGlobalBounds();

		for (const auto& platform : platforms) {
			sf::FloatRect platformBounds = platform.shape.getGlobalBounds();

			if (platform.oneWay) {
				if (velocity.y < 0.f) continue; // going up -> ignores
				if (dropping) continue;         // dropping -> ignores
			}

			bool wasAbove = (previousPosition.y + height) <= platformBounds.position.y + 2.f;

			if (playerBounds.findIntersection(platformBounds)) {
				if (velocity.y > 0.f && wasAbove) {
					body.setPosition({ body.getPosition().x, platformBounds.position.y - height });
					velocity.y = 0.f;
					onGround = true;
					jumpsRemaining = maxJumps;
					playerBounds = body.getGlobalBounds();
				}
			}
		}

		// ----------------------------
		// Drop-through + Jump (disabled in hitstun)
		// ----------------------------
		if (!inHitstun()) {
			// Drop-Through (S while grounded)
			if (onGround && sf::Keyboard::isKeyPressed(controls.down)) {
				dropping = true;
				dropTimer = dropDuration;
				onGround = false;
			}

			if (dropping) {
				dropTimer -= dt;
				if (dropTimer <= 0.f) {
					dropping = false;
				}
			}

			// Jump
			bool jumpDown = sf::Keyboard::isKeyPressed(controls.jump);
			bool jumpPressed = jumpDown && !jumpWasDown;
			jumpWasDown = jumpDown;

			if (jumpPressed && jumpsRemaining > 0) {
				velocity.y = jumpForce;
				onGround = false;
				jumpsRemaining--; 
			}
		}

		// ----------------------------
		// Hitstun Timer
		// ----------------------------
		if (hitstunTimer > 0.f) {
			hitstunTimer -= dt;
			if (hitstunTimer < 0.f) hitstunTimer = 0.f;
		}

		//------------------------------
		//Knockdown Timer
		//------------------------------
		bool nowDown = (knockdownTimer > 0.f);

		if (nowDown) {
			knockdownTimer -= dt;
			if (knockdownTimer < 0.f) { knockdownTimer = 0.f; }

			//while downed, cannot defend and stop movement
			isBlocking = false;
			stopRun();
			if (isDodging) { stopDodge(); }
		}

		// Detect knockdown ending this frame -> start get-up lock
		bool endedThisFrame = (wasKnockedDown && knockdownTimer <= 0.f);
		if (endedThisFrame) {
			knockdownLockTimer = knockdownLockMin;
		}
		wasKnockedDown = (knockdownTimer > 0.f);

		//Tick lock timer
		if (knockdownLockTimer > 0.f) {
			knockdownLockTimer -= dt;
			if (knockdownLockTimer < 0.f) { knockdownLockTimer = 0.f; }
		}
	}

	// ----------------------------
	// Rendering
	// ----------------------------
	void draw(sf::RenderWindow& window) const {
		if (!alive) return;

		// tints invulnerable
		sf::Color c = body.getFillColor();
		c.a = isInvulnerable() ? 160 : 255;

		sf::RectangleShape copy = body;
		copy.setFillColor(c);
		window.draw(copy);
	}
};