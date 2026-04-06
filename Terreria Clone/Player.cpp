#include "Player.h"
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>

Player::Player() {
	body.setSize({ width, height });
	body.setFillColor(sf::Color::Blue);
	body.setPosition({ -100.f, 100.f });

	lightChain[0] = MoveData{
		.startup = 0.04f, .active = 0.06f, .recovery = 0.16f,
		.hitboxSize = {55.f, 22.f},
		.hitboxOffset = {width, height * 0.35f},
		.knockback = {35.f, -8.f},
		.knockbackScaling = 0.08f,
		.damage = 2.5f,
		.hitstun = 0.28f,
		.cancelStart = 0.04f + 0.06f,
		.cancelEnd = (0.04f + 0.06f) + 0.16f * HEAVY_CANCEL_RECOVERY_FRACTION,
		.cancelOnHitOnly = true,
		.lungeSpeed = 0.f,
		.lungeDuration = 0.f
	};

	lightChain[1] = MoveData{
		.startup = 0.05f, .active = 0.06f, .recovery = 0.16f,
		.hitboxSize = {60.f, 22.f},
		.hitboxOffset = {width, height * 0.33f},
		.knockback = {45.f, -10.f},
		.knockbackScaling = 0.10f,
		.damage = 3.0f,
		.hitstun = 0.30f,
		.cancelStart = 0.05f + 0.06f,
		.cancelEnd = (0.05f + 0.06f) + 0.16f * HEAVY_CANCEL_RECOVERY_FRACTION,
		.cancelOnHitOnly = true
	};

	lightChain[2] = MoveData{
		.startup = 0.06f, .active = 0.07f, .recovery = 0.14f,
		.hitboxSize = {65.f, 24.f},
		.hitboxOffset = {width + 8.f, height * 0.30f},
		.knockback = {170.f, -45.f},
		.knockbackScaling = 0.55f,
		.damage = 4.5f,
		.hitstun = 0.33f,
		.cancelStart = 0.06f + 0.07f,
		.cancelEnd = (0.06f + 0.07f) + 0.14f * HEAVY_CANCEL_RECOVERY_FRACTION,
		.cancelOnHitOnly = true
	};

	heavyMove = MoveData{
		.startup = 0.10f, .active = 0.08f, .recovery = 0.22f,
		.hitboxSize = {70.f, 28.f},
		.hitboxOffset = {width, height * 0.25f},
		.knockback = {260.f, -120.f},
		.knockbackScaling = 0.95f,
		.damage = 7.0f,
		.hitstun = 0.35f,
	};

	runLightMove = MoveData{
		.startup = 0.06f, .active = 0.08f, .recovery = 0.22f,
		.hitboxSize = {75.f, 20.f},
		.hitboxOffset = {width, height * 0.72f},
		.knockback = {95.f, -8.f},
		.knockbackScaling = 0.20f,
		.damage = 4.0f,
		.hitstun = 0.30f,
		.causesKnockdown = false,
		.cancelStart = 0.f,
		.cancelEnd = 0.f,
		.cancelOnHitOnly = true,
		.slideSpeed = 150.f,
		.slideDuration = 0.045f
	};
}

bool Player::isAttacking() const {
	return attack.current != AttackID::None;
}

const MoveData* Player::currentMoveData() const {
	switch (attack.current) {
	case AttackID::Light1: return &lightChain[0];
	case AttackID::Light2: return &lightChain[1];
	case AttackID::Light3: return &lightChain[2];
	case AttackID::Heavy: return &heavyMove;
	case AttackID::RunLight: return &runLightMove;
	default: return nullptr;
	}
}

bool Player::isLightAttack(AttackID id) const {
	return id == AttackID::Light1 || id == AttackID::Light2 || id == AttackID::Light3;
}

float Player::attackElapsed() const {
	const MoveData* m = currentMoveData();
	if (!m) { return 0.f; }
	float total = m->startup + m->active + m->recovery;
	return total - attack.timer;
}

bool Player::moveIsInActiveFrames() const {
	const MoveData* m = currentMoveData();
	if (!m) { return false; }

	float total = m->startup + m->active + m->recovery;
	float elapsed = total - attack.timer;

	return elapsed >= m->startup && elapsed < (m->startup + m->active);
}

void Player::resetCombo() {
	comboCount = 0;
	comboTimer = 0.f;
}

void Player::registerComboHit() {
	comboCount++;
	comboTimer = comboResetTime;
}

void Player::applyAttackLunge() {
	if (!alive) { return; }
	if (inHitstun()) { return; }
	if (!isAttacking()) { return; }

	const MoveData* m = currentMoveData();
	if (!m) { return; }

	if (m->lungeSpeed <= 0.f) { return; }
	if (m->lungeDuration <= 0.f) { return; }

	float e = attackElapsed();
	if (e > m->lungeDuration) { return; }

	float dir = facingRight ? 1.f : -1.f;
	velocity.x = dir * m->lungeSpeed;
}

bool Player::canCancelToHeavy() const {
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

bool Player::canChainToNextLight() const {
	if (!isAttacking()) { return false; }

	if (!(attack.current == AttackID::Light1 || attack.current == AttackID::Light2)) {
		return false;
	}

	const MoveData* m = currentMoveData();
	if (!m) { return false; }

	if (!attack.hasHit) { return false; }

	float e = attackElapsed();
	float recoveryStart = m->startup + m->active;
	float chainEnd = recoveryStart + (m->recovery * 0.85f);

	return e >= recoveryStart && e <= chainEnd;
}

bool Player::inKnockdown() const {
	return knockdownTimer > 0.f;
}

bool Player::movementLocked() const {
	return inHitstun() || inKnockdown() || knockdownLockTimer > 0.f || isAttacking() || blockstunTimer > 0.f || landingLagTimer > 0.f;
}

bool Player::actionsLocked() const {
	return inHitstun() || inKnockdown() || knockdownLockTimer > 0.f || isDodging || isBlocking || blockstunTimer > 0.f || landingLagTimer > 0.f;
}

bool Player::canSpendBufferedInputs() const {
	return !actionsLocked();
}

void Player::stopRun() {
	isRunning = false;
	runDir = 0;
	runActiveTimer = 0.f;
	runStopGraceTimer = 0.f;
	leftTapTimer = 0.f;
	rightTapTimer = 0.f;
}

void Player::stopDodge() {
	isDodging = false;
	dodgeDir = 0;
	dodgeTimer = 0.f;
	dodgeInvulnTimer = 0.f;
}

sf::FloatRect Player::makeHitboxRect(const MoveData& m) const {
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

sf::FloatRect Player::currentHitbox() const {
	const MoveData* m = currentMoveData();
	if (!m) { return sf::FloatRect({}, {}); }
	if (!moveIsInActiveFrames()) { return sf::FloatRect({}, {}); }
	return makeHitboxRect(*m);
}

Player::AttackPhase Player::attackPhase() const {
	const MoveData* m = currentMoveData();
	if (!m) { return AttackPhase::None; }

	float total = m->startup + m->active + m->recovery;
	float elapsed = total - attack.timer;

	if (elapsed < 0.f) { return AttackPhase::None; }
	if (elapsed < m->startup) { return AttackPhase::Startup; }
	if (elapsed < (m->startup + m->active)) { return AttackPhase::Active; }
	if (elapsed < (m->startup + m->active + m->recovery)) { return AttackPhase::Recovery; }

	return AttackPhase::None;
}

sf::FloatRect Player::plannedHitbox() const {
	const MoveData* m = currentMoveData();
	if (!m) { return sf::FloatRect({}, {}); }
	return makeHitboxRect(*m);
}

sf::Vector2f Player::center() const {
	return {
		body.getPosition().x + body.getSize().x * 0.5f,
		body.getPosition().y + body.getSize().y * 0.5f
	};
}

void Player::addDamage(float amount) {
	if (amount <= 0.f) { return; }
	damagePercent += amount;
	if (damagePercent > maxDamagePercent) {
		damagePercent = maxDamagePercent;
	}
}

void Player::resetDamage() {
	damagePercent = 0.0f;
}

float Player::getDamage() const {
	return damagePercent;
}

bool Player::canBlock() const {
	return !shieldBroken && shield > 0.f;
}

void Player::resetShield() {
	shield = maxShield;
	shieldBroken = false;
	shieldBreakTimer = 0.f;
	shieldRegenDelayTimer = 0.f;
}

void Player::setSpawnPoint(sf::Vector2f p) {
	spawnPoint = p;
}

void Player::resetForStageStart(sf::Vector2f spawnPos) {
	setSpawnPoint(spawnPos);
	respawnNow();
	dropping = false;
	dropTimer = 0.f;
}

bool Player::isInvulnerable() const {
	return spawnInvulnTimer > 0.f;
}

bool Player::canBeHit() const {
	return alive && !isInvulnerable() && !(dodgeInvulnTimer > 0.f);
}

void Player::kill() {
	if (!alive) return;
	alive = false;
	respawnTimer = respawnDelay;
	velocity = { 0.f, 0.f };
}

void Player::respawnNow() {
	body.setPosition(spawnPoint);
	velocity = { 0.f, 0.f };
	alive = true;

	onGround = false;
	dropping = false;
	dropTimer = 0.f;

	stopRun();
	stopDodge();
	isBlocking = false;
	blockstunTimer = 0.f;

	attack.current = AttackID::None;
	attack.timer = 0.f;
	attack.hasHit = false;

	lightBufferTimer = 0.f;
	heavyBufferTimer = 0.f;
	hitstopTimer = 0.f;
	queuedAttack = AttackID::None;

	resetDamage();
	resetCombo();
	resetShield();

	spawnInvulnTimer = spawnInvulnDuration;
}

bool Player::inHitstun() const {
	return hitstunTimer > 0.f || knockdownTimer > 0.f;
}

void Player::startHitstop(float seconds) {
	hitstopTimer = seconds;
}

sf::FloatRect Player::hurtbox() const {
	return body.getGlobalBounds();
}

void Player::takeHit(const sf::Vector2f& baseKnockback, float hitstunSeconds, float damage, float knockbackScaling, bool knockdown, float knockdownSeconds) {
	if (!canBeHit()) return;

	float percentScale = 1.0f + (damagePercent / 100.0f) * knockbackScaling;
	percentScale *= (1.0f + damagePercent / 300.f);
	sf::Vector2f scaledKnockback = {
		baseKnockback.x * percentScale,
		baseKnockback.y * percentScale
	};

	float comboDecay = 1.0f - (comboCount * 0.08f);
	if (comboDecay < 0.72f) { comboDecay = 0.72f; }

	float percentBonus = 1.0f + (damagePercent * 0.002f);
	if (percentBonus > 1.25f) { percentBonus = 1.25f; }

	float finalHitstun = hitstunSeconds * comboDecay * percentBonus;
	if (finalHitstun < 0.18f) { finalHitstun = 0.18f; }

	if (isBlocking) {
		float blockedDamage = damage * 0.35f;
		addDamage(blockedDamage);

		shield -= damage * 3.0f;
		if (shield < 0.f) { shield = 0.f; }
		shieldRegenDelayTimer = shieldRegenDelay;
		if (shield <= 0.f) {
			shieldBroken = true;
			shieldBreakTimer = shieldBreakDuration;
			blockstunTimer = shieldBreakDuration;
			isBlocking = false;
			stopRun();
			stopDodge();
		}

		sf::Vector2f reducedKB{ scaledKnockback.x * 0.25f, scaledKnockback.y * 0.15f };
		float reducedBlockstun = finalHitstun * 0.65f;
		if (reducedBlockstun < 0.10f) { reducedBlockstun = 0.10f; }

		knockdown = false;
		knockdownSeconds = 0.f;

		blockstunTimer = reducedBlockstun;
		hitstunTimer = 0.f;
		knockdownTimer = 0.f;
		isBlocking = false;
		velocity = reducedKB;
		onGround = false;
		return;
	}

	addDamage(damage);

	stopDodge();
	stopRun();

	attack.current = AttackID::None;
	attack.timer = 0.f;
	attack.hasHit = false;

	chainIndex = -1;
	chainTimer = 0.f;
	queuedAttack = AttackID::None;
	lightBufferTimer = 0.f;
	heavyBufferTimer = 0.f;

	hitstunTimer = finalHitstun;

	if (knockdown) {
		knockdownTimer = knockdownSeconds;
		stopRun();
		stopDodge();
		isBlocking = false;
	}
	else {
		knockdownTimer = 0.f;
	}

	velocity = scaledKnockback;
	onGround = false;
}

void Player::startAttack(AttackID id) {
	if (!alive) return;
	if (inHitstun()) return;
	if (isAttacking()) return;

	attack.current = id;
	attack.hasHit = false;
	lastAttackHit = false;

	if (id == AttackID::RunLight) {
		stopRun();
	}

	bool holdLeft = sf::Keyboard::isKeyPressed(controls.left);
	bool holdRight = sf::Keyboard::isKeyPressed(controls.right);

	if (id == AttackID::Light1) {
		attack.variant = (holdLeft || holdRight) ? AttackVariant::Side : AttackVariant::Neutral;
	}
	else if (id == AttackID::Light2 || id == AttackID::Light3) {
		attack.variant = AttackVariant::Neutral;
	}
	else {
		attack.variant = (holdLeft || holdRight) ? AttackVariant::Side : AttackVariant::Neutral;
	}

	bool preserveMomentum =
		(id == AttackID::RunLight) ||
		(id == AttackID::Heavy) ||
		(id == AttackID::Light1 && attack.variant == AttackVariant::Side);

	if (!preserveMomentum && velocity.y == 0.f) {
		velocity.x = 0.f;
	}

	const MoveData* m = currentMoveData();
	attack.timer = m ? (m->startup + m->active + m->recovery) : 0.f;
}

void Player::startNextLightChain() {
	if (chainIndex < 0) {
		chainIndex = 0;
	}
	else if (chainIndex < 2) {
		chainIndex++;
	}
	else {
		chainIndex = -1;
		return;
	}

	AttackID id =
		(chainIndex == 0) ? AttackID::Light1 :
		(chainIndex == 1) ? AttackID::Light2 :
		AttackID::Light3;

	startAttack(id);
}


// ------------------------
// UPDATE
// ------------------------
void Player::update(float dt, const std::vector<Platform>& platforms) {
	// Reset per-frame state
	bool wasGroundedAtFrameStart = onGround;
	bool startedFrameAirborne = !onGround;
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

	if (lightBufferTimer <= 0.f && heavyBufferTimer <= 0.f) {
		queuedAttack = AttackID::None;
	}

	//-----------------------------
	// Landing Lag Timer
	//-----------------------------
	if (landingLagTimer > 0.f) {
		landingLagTimer -= dt;
		if (landingLagTimer < 0.f) { landingLagTimer = 0.f; }
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

			// record hit-confirm result for debugging / fallback logic
			if (isLightAttack(finished) && finishedHit) {
				chainTimer = chainWindow;

				// if player did NOT already buffer another light, reset chain
				if (!(queuedAttack == AttackID::Light1 && lightBufferTimer > 0.f)) {
					chainIndex = -1;
				}
			}
			else {
				chainTimer = 0.f;
				chainIndex = -1;
			}
		}
	}

	//--------------------------
	// Hitscaling Timer
	//--------------------------
	if (comboTimer > 0.f) {
		comboTimer -= dt;
		if (comboTimer <= 0.f) {
			comboTimer = 0.f;
			comboCount = 0;
		}
	}

	//--------------------------
	// Shield Break Timer
	//--------------------------
	if (shieldBreakTimer > 0.f) {
		shieldBreakTimer -= dt;
		if (shieldBreakTimer < 0.f) { shieldBreakTimer = 0.f; }
		if (shieldBreakTimer == 0.f) {
			shieldBroken = false;
		}
	}

	if (shieldRegenDelayTimer > 0.f) {
		shieldRegenDelayTimer -= dt;
		if (shieldRegenDelayTimer < 0.f) { shieldRegenDelayTimer = 0.f; }
	}
	else if (!isBlocking && !shieldBroken && shield < maxShield) {
		shield += shieldRegenRate * dt;
		if (shield > maxShield) { shield = maxShield; }
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
	// Run Active Timer
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

	if (blockstunTimer > 0.f) {
		blockstunTimer -= dt;
		if (blockstunTimer < 0.f) { blockstunTimer = 0.f; }
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

	if (lightPressed) {
		lightBufferTimer = inputBufferWindow;
		queuedAttack = AttackID::Light1;
	}
	if (heavyPressed) {
		heavyBufferTimer = inputBufferWindow;
		queuedAttack = AttackID::Heavy;
	}

	// ----------------------------
	// Spend buffered inputs (priority: heavy then light)
	// ----------------------------
	if (queuedAttack != AttackID::None) {

		// Heavy intent
		if (queuedAttack == AttackID::Heavy && heavyBufferTimer > 0.f) {
			if (canSpendBufferedInputs()) {
				if (isAttacking() && canCancelToHeavy()) {
					attack.current = AttackID::None;
					attack.timer = 0.f;
					attack.hasHit = false;

					chainIndex = -1;
					chainTimer = 0.f;

					heavyBufferTimer = 0.f;
					queuedAttack = AttackID::None;
					startAttack(AttackID::Heavy);
				}
				else if (!isAttacking()) {
					chainIndex = -1;
					chainTimer = 0.f;

					heavyBufferTimer = 0.f;
					queuedAttack = AttackID::None;
					startAttack(AttackID::Heavy);
				}
			}
		}

		// Light intent
		else if (queuedAttack == AttackID::Light1 && lightBufferTimer > 0.f) {

			// Start fresh
			if (canSpendBufferedInputs() && !isAttacking()) {
				lightBufferTimer = 0.f;
				queuedAttack = AttackID::None;

				bool canRunSlide = (isRunning && runActiveTimer > 0.f);
				if (canRunSlide) {
					chainIndex = -1;
					chainTimer = 0.f;
					startAttack(AttackID::RunLight);
				}
				else {
					startNextLightChain();
				}
			}

			//Chain only on hit-confirm during valid window
			else if (isAttacking() && canChainToNextLight()) {
				lightBufferTimer = 0.f;
				queuedAttack = AttackID::None;

				attack.current = AttackID::None;
				attack.timer = 0.f;
				attack.hasHit = false;

				startNextLightChain();
			}
		}
	}

	// ----------------------------
	// Movement input (disabled in hitstun/attacking)
	// ----------------------------
	if (!movementLocked()) {
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
		if (!isDodging && defenseDown && sideDir == 0 && canBlock()) {
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

		//----------------------------
		// Running state logic
		//----------------------------
		if (dir == 0) {
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
			runStopGraceTimer = runStopGraceMax;

			if (isRunning) {
				runDir = dir;
			}
		}

		//----------------------------
		// Horizontal movement
		//----------------------------
		if (wasGroundedAtFrameStart) {

			if (dir != 0) {
				float speed = isRunning ? runSpeed : walkSpeed;
				if (isRunning) { runActiveTimer = runActiveMin; }

				velocity.x = dir * speed;
				facingRight = (dir > 0);
			}
			else {
				float friction = isRunning ? runGroundFriction : groundFriction;

				if (velocity.x > 0.f) {
					velocity.x -= friction * dt;
					if (velocity.x < 0.f) velocity.x = 0.f;
				}
				else if (velocity.x < 0.f) {
					velocity.x += friction * dt;
					if (velocity.x > 0.f) velocity.x = 0.f;
				}
			}
		}
		else {
			if (dir != 0) {
				velocity.x += dir * airAccel * dt;

				//Only clamp if player is trying to push BEYOND current carried momentum
				if (dir > 0 && velocity.x > airMaxSpeed && velocity.x < runSpeed) {
					velocity.x = airMaxSpeed;
				}
				if (dir < 0 && velocity.x < -airMaxSpeed && velocity.x > -runSpeed) {
					velocity.x = -airMaxSpeed;
				}

				//Preserve stronger takeoff momentum from running
				if (velocity.x > runSpeed) velocity.x = runSpeed;
				if (velocity.x < -runSpeed) velocity.x = -runSpeed;

				facingRight = (dir > 0);
			}
			else {
				if (velocity.x > 0.f) {
					velocity.x -= airDecel * dt;
					if (velocity.x < 0.f) velocity.x = 0.f;
				}
				else if (velocity.x < 0.f) {
					velocity.x += airDecel * dt;
					if (velocity.x > 0.f) velocity.x = 0.f;
				}
			}
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
	// Stop unwanted horizontal override during attacks
	//-----------------------------
	if (!inHitstun() && isAttacking()) {
		bool allowSideLightCarry =
			(attack.current == AttackID::Light1 && attack.variant == AttackVariant::Side);
		bool allowRunLightCarry =
			(attack.current == AttackID::RunLight);
		bool allowHeavyCarry =
			(attack.current == AttackID::Heavy);


		// use frame-start grounded state, not curent onGround
		if (!allowSideLightCarry && !allowRunLightCarry && !allowHeavyCarry) {
			if (wasGroundedAtFrameStart) {
				velocity.x = 0.f;
			}
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
	bool jumpHeldNow = sf::Keyboard::isKeyPressed(controls.jump);

	float appliedGravity = gravity;

	if (!onGround && velocity.y < 0.f) {
		if (jumpHeldNow && jumpHoldTimer > 0.f) {
			appliedGravity *= jumpHoldGravityMultiplier;
			jumpHoldTimer -= dt;
			if (jumpHoldTimer < 0.f) { jumpHoldTimer = 0.f; }
		}
		else {
			appliedGravity *= jumpReleaseGravityMultiplier;
		}
	}

	velocity.y += dt * appliedGravity;

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
	if (fastFalling) {
		velocity.y += fastFallGravityBonus * dt;
	}


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
				bool landedThisFrame = !onGround;
				onGround = true;
				jumpsRemaining = maxJumps;
				fastFalling = false;
				jumpHoldTimer = 0.f;

				if (landedThisFrame && startedFrameAirborne) {
					if (attack.current == AttackID::Heavy) {
						landingLagTimer = landingLagHeavy;
					}
					else if (isAttacking()) {
						landingLagTimer = landingLagLight;
					}
				}
				playerBounds = body.getGlobalBounds();
			}
		}
	}

	// ----------------------------
	// Drop-through + Jump (disabled in hitstun)
	// ----------------------------
	if (!inHitstun()) {
		// Fast Fall
		bool downHeld = sf::Keyboard::isKeyPressed(controls.down);

		if (!onGround && !dropping && velocity.y > 0.f && downHeld && !fastFalling) {
			fastFalling = true;

			// Snap into clearly faster fall immediately
			if (velocity.y < fastFallSpeed) {
				velocity.y = fastFallSpeed;
			}
		}

		// Drop-Through (S while grounded)
		if (onGround && downHeld) {
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
			jumpHoldTimer = maxJumpHoldTime;
			fastFalling = false;
		}
	}

	// ----------------------------
	// Hitstun Timer
	// ----------------------------
	if (hitstunTimer > 0.f) {
		hitstunTimer -= dt;
		if (hitstunTimer < 0.f) hitstunTimer = 0.f;
	}
	/*
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
	}*/

	//------------------------
	// Horizontal Drag
	//------------------------
	if ((hitstunTimer > 0.f || knockdownTimer > 0.f || blockstunTimer > 0.f) && onGround) {
		if (velocity.x > 0.f) {
			velocity.x -= groundFriction * dt;
			if (velocity.x < 0.f) { velocity.x = 0.f; }
		}
		else if (velocity.x < 0.f) {
			velocity.x += groundFriction * dt;
			if (velocity.x > 0.f) { velocity.x = 0.f; }
		}
	}
}

// ----------------
// RENDER
// ________________

void Player::draw(sf::RenderWindow& window, bool showStateTints) const {
	if (!alive) return;

	sf::RectangleShape copy = body;
	sf::Color c = body.getFillColor();

	if (showStateTints) {
		//Priority order
		if (knockdownTimer > 0.f || knockdownLockTimer > 0.f) {
			c = sf::Color(255, 140, 0); //orange
		}
		else if (isBlocking) {
			c = sf::Color(80, 160, 255); //blue
		}
		else if (dodgeInvulnTimer > 0.f || isDodging) {
			c = sf::Color(180, 0, 255); //purple
		}
		else if (hitstunTimer > 0.f) {
			c = sf::Color(255, 120, 120);//light red
		}
	}

	if (isInvulnerable()) {
		c.a = 160;
	}
	else {
		c.a = 255;
	}

	copy.setFillColor(c);
	window.draw(copy);
}


