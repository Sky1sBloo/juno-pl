# Standard Library Reference

Qualified calls use the form `<Qualifier>.<name>(<args>)`. Each function is
registered as either an **instruction** (used with `perform`) or an
**expression** (used inside expressions), with an accepted argument count
range.

Some names exist in both contexts (e.g. `Input.throttle(x)` as an
instruction, `Input.throttle()` as an expression).

## Program Flow — `Game` (instructions)

| Function | Signature | Description |
|----------|-----------|-------------|
| `Game.delay` | `(seconds)` | Waits the given number of seconds. |
| `Game.delayUntil` | `(condition)` | Waits until the given mission condition. |
| `Game.timeMode` | `(mode)` | Sets time warp mode. |
| `Game.switchToCraft` | `(craftId)` | Switches active craft. |
| `Game.beep` | `(freq, dur, vol)` | Plays a beep sound. |
| `Game.flightLog` | `(text, overwrite)` | Writes a message to the flight log.  |
| `Game.display` | `(text)` | Shows a message on screen. |

## Math — `Math` (expressions)

| Function | Signature | Description |
|----------|-----------|-------------|
| `Math.random` | `(min, max)` | Random number in range. |
| `Math.min` | `(a, b)` | Smaller of two values. |
| `Math.max` | `(a, b)` | Larger of two values. |
| `Math.not` | `(a)` | Logical NOT. |
| `Math.abs` | `(x)` | Absolute value. |
| `Math.floor` | `(x)` | Round down. |
| `Math.ceil` | `(x)` | Round up. |
| `Math.round` | `(x)` | Round to nearest. |
| `Math.sqrt` | `(x)` | Square root. |
| `Math.sin` | `(x)` | Sine (radians). |
| `Math.cos` | `(x)` | Cosine (radians). |
| `Math.tan` | `(x)` | Tangent (radians). |
| `Math.asin` | `(x)` | Arcsine (radians). |
| `Math.acos` | `(x)` | Arccosine (radians). |
| `Math.atan` | `(x)` | Arctangent (radians). |
| `Math.ln` | `(x)` | Natural logarithm. |
| `Math.log` | `(x)` | Base-10 logarithm. |
| `Math.deg2rad` | `(deg)` | Degrees to radians. |
| `Math.rad2deg` | `(rad)` | Radians to degrees. |
| `Math.atan2` | `(y, x)` | Arctangent of y/x. |

## Strings — `String` / `Strings` (expressions)

| Function | Signature | Description |
|----------|-----------|-------------|
| `String.join` | `(a, b, ...)` | Joins 2+ strings with `,` separator (variadic). |
| `String.lengthOf` | `(s)` | Length of a string. |
| `String.letterOf` | `(s, i)` | Character at index i. |
| `String.substr` | `(s, start, len)` | Substring. |
| `String.strContains` | `(s, sub)` | Whether s contains sub. |
| `String.format` | `(template, ...)` | Format string with substitutions (variadic). |
| `String.friendly` | `(a, b)` | Friendly comparison/formatting. |

## Vectors — `Vector` (expressions)

| Function | Signature | Description |
|----------|-----------|-------------|
| `Vector.vec` | `(x, y, z)` | Construct a vector. |
| `Vector.vecLength` | `(v)` | Vector magnitude. |
| `Vector.vecX` | `()` | X component of last vector context. |
| `Vector.vecY` | `()` | Y component. |
| `Vector.vecZ` | `()` | Z component. |
| `Vector.normalize` | `(v)` | Normalized vector. |
| `Vector.angle` | `(a, b)` | Angle between two vectors. |
| `Vector.clamp` | `(v, min, max)` | Clamp vector components. |
| `Vector.cross` | `(a, b)` | Cross product. |
| `Vector.dot` | `(a, b)` | Dot product. |
| `Vector.dist` | `(a, b)` | Distance between two vectors. |
| `Vector.min` | `(a, b)` | Component-wise minimum. |
| `Vector.max` | `(a, b)` | Component-wise maximum. |
| `Vector.project` | `(a, b)` | Project a onto b. |
| `Vector.scale` | `(v, s)` | Scale vector by scalar. |

## Game expressions — `Game` (expressions)

| Function | Signature | Description |
|----------|-----------|-------------|
| `Game.funk` | `(x)` | Funk transformation. |
| `Game.frequencyOf` | `(note, octave)` | Frequency for a musical note. |
| `Game.speak` | `(text, pitch, speed)` | Speak text aloud. |
| `Game.sineBeep` | `(freq, dur, vol)` | Sine-wave beep. |
| `Game.sineTriangle` | `(freq, dur, vol)` | Triangle-wave beep. |
| `Game.sineSaw` | `(freq, dur, vol)` | Saw-wave beep. |
| `Game.sineSquare` | `(freq, dur, vol)` | Square-wave beep. |
| `Game.sineNoise` | `(freq, dur, vol)` | Noise beep. |

## Craft instructions — `Craft` (instructions)

| Function | Signature | Description |
|----------|-----------|-------------|
| `Craft.pitchTo` | `(degrees)` | Pitch craft to angle. |
| `Craft.headingTo` | `(degrees)` | Heading to angle. |
| `Craft.setPitchPID` | `(p, i, d)` | Set pitch PID gains. |
| `Craft.setRollPID` | `(p, i, d)` | Set roll PID gains. |
| `Craft.setTarget` | `(target)` | Set target. |
| `Craft.lockPrograde` | `()` | Lock to prograde. |
| `Craft.lockRetrograde` | `()` | Lock to retrograde. |
| `Craft.lockNone` | `()` | Clear lock. |
| `Craft.lockTarget` | `()` | Lock to target. |
| `Craft.lockBurnNode` | `()` | Lock to burn node. |
| `Craft.lockCurrent` | `()` | Lock to current attitude. |
| `Craft.lockVector` | `(x, y, z)` | Lock to vector. |
| `Craft.activateStage` | `()` | Activate the next stage. |

## Input instructions — `Input` (instructions)

| Function | Signature | Description |
|----------|-----------|-------------|
| `Input.throttle` | `(value)` | Set throttle. |
| `Input.pitch` | `(value)` | Set pitch input. |
| `Input.roll` | `(value)` | Set roll input. |
| `Input.yaw` | `(value)` | Set yaw input. |
| `Input.brake` | `(value)` | Set brake. |
| `Input.slider` | `(index, value)` | Set slider value. |
| `Input.translateForward` | `(value)` | Forward translation. |
| `Input.translateRight` | `(value)` | Right translation. |
| `Input.translateUp` | `(value)` | Up translation. |
| `Input.activationGroup` | `(group, state)` | Toggle activation group. |
| `Input.translationMode` | `(mode)` | Set translation mode. |

## Part instructions — `Part` (instructions)

| Function | Signature | Description |
|----------|-----------|-------------|
| `Part.activate` | `(partId, state)` | Activate/deactivate a part. |
| `Part.focus` | `(partId, state)` | Focus/unfocus a part. |
| `Part.unfocus` | `(partId)` | Unfocus a part. |
| `Part.name` | `(partId, name)` | Rename a part. |
| `Part.detach` | `(partId)` | Detach a part. |
| `Part.explode` | `(partId)` | Explode a part. |
| `Part.drainFuel` | `(partId)` | Drain fuel from a part. |
| `Part.fillFuel` | `(partId)` | Fill fuel in a part. |

## Game actions — `Game` (instructions)

| Function | Signature | Description |
|----------|-----------|-------------|
| `Game.setVariable` | `(name, value)` | Set a Vizzy variable (input qualifier context). |

## Camera controls — `Camera` (instructions)

| Function | Signature | Description |
|----------|-----------|-------------|
| `Camera.zoom` | `(level)` | Set zoom level. |
| `Camera.xRotation` | `(deg)` | X rotation. |
| `Camera.yRotation` | `(deg)` | Y rotation. |
| `Camera.tilt` | `(deg)` | Tilt angle. |
| `Camera.mode` | `(mode)` | Camera mode. |
| `Camera.targetOffset` | `(x, y, z)` or `(x, y, z)` | Target offset (1 or 3 args). |

## List operations — `Lists` (instructions)

| Function | Signature | Description |
|----------|-----------|-------------|
| `Lists.insert` | `(ident, value, ...)` | Insert values into a list (variadic, 2+). |
| `Lists.set` | `(ident, index, value)` | Set list element at index. |
| `Lists.remove` | `(ident, index)` | Remove element at index. |
| `Lists.clear` | `(ident)` | Clear a list. |
| `Lists.sort` | `(ident)` | Sort a list. |
| `Lists.reverse` | `(ident)` | Reverse a list. |

## List operations — `Lists` (expressions)

| Function | Signature | Description |
|----------|-----------|-------------|
| `Lists.length` | `(list)` | Number of elements. |
| `Lists.idxOf` | `(list, value)` | Index of a value. |
| `Lists.index` | `(list, value)` | Index of a value. |

## Craft information — expressions

### `Altitude`
| Function | Signature | Description |
|----------|-----------|-------------|
| `Altitude.getAGL` | `()` | Height above ground. |
| `Altitude.getASL` | `()` | Height above sea level. |
| `Altitude.getASF` | `()` | Height above surface. |

### `Orbit`
| Function | Signature | Description |
|----------|-----------|-------------|
| `Orbit.ap` | `()` | Apoapsis. |
| `Orbit.pe` | `()` | Periapsis. |
| `Orbit.timeToAp` | `()` | Time to apoapsis. |
| `Orbit.timeToPe` | `()` | Time to periapsis. |
| `Orbit.eccentricity` | `()` | Orbital eccentricity. |
| `Orbit.inclination` | `()` | Orbital inclination. |
| `Orbit.period` | `()` | Orbital period. |
| `Orbit.planetName` | `()` | Current planet name. |

### `Atmosphere`
| Function | Signature | Description |
|----------|-----------|-------------|
| `Atmosphere.density` | `()` | Air density. |
| `Atmosphere.pressure` | `()` | Air pressure. |
| `Atmosphere.speedOfSound` | `()` | Speed of sound. |
| `Atmosphere.temperature` | `()` | Air temperature. |
| `Atmosphere.elements` | `()` | Atmospheric elements. |
| `Atmosphere.composition` | `()` | Atmospheric composition. |

### `Performance`
| Function | Signature | Description |
|----------|-----------|-------------|
| `Performance.engineThrust` | `()` | Current engine thrust. |
| `Performance.mass` | `()` | Total mass. |
| `Performance.dryMass` | `()` | Dry mass. |
| `Performance.fuelMass` | `()` | Fuel mass. |
| `Performance.maxEngineThrust` | `()` | Maximum engine thrust. |
| `Performance.twr` | `()` | Thrust-to-weight ratio. |
| `Performance.currentISP` | `()` | Current specific impulse. |
| `Performance.stageDeltaV` | `()` | Stage delta-v. |
| `Performance.stageBurnTime` | `()` | Stage burn time. |

### `Fuel`
| Function | Signature | Description |
|----------|-----------|-------------|
| `Fuel.battery` | `()` | Battery charge. |
| `Fuel.stageFuel` | `()` | Stage fuel. |
| `Fuel.monoProp` | `()` | Monopropellant. |
| `Fuel.totalFuel` | `()` | Total fuel. |

## Navigation — expressions

### `Nav`
| Function | Signature | Description |
|----------|-----------|-------------|
| `Nav.nav` | `()` | Navigation data. |
| `Nav.target` | `()` | Target data. |
| `Nav.heading` | `()` | Current heading. |
| `Nav.pitch` | `()` | Current pitch. |
| `Nav.autopilotHeading` | `()` | Autopilot heading. |
| `Nav.autopilotPitch` | `()` | Autopilot pitch. |
| `Nav.bankAngle` | `()` | Bank angle. |
| `Nav.angleOfAttack` | `()` | Angle of attack. |
| `Nav.sideSlip` | `()` | Side slip. |
| `Nav.northVector` | `()` | North vector. |
| `Nav.eastVector` | `()` | East vector. |
| `Nav.rollAxis` | `()` | Roll axis vector. |
| `Nav.pitchAxis` | `()` | Pitch axis vector. |
| `Nav.yawAxis` | `()` | Yaw axis vector. |
| `Nav.craftRight` | `()` | Craft right vector. |
| `Nav.craftUp` | `()` | Craft up vector. |
| `Nav.toLatLongAGL` | `(pos)` | Position to lat/long AGL. |
| `Nav.toLatLongASL` | `(pos)` | Position to lat/long ASL. |
| `Nav.toPosition` | `(lat, long)` | Lat/long to position. |
| `Nav.toPositionOverSea` | `(lat, long)` | Lat/long over sea to position. |
| `Nav.getHeightAtLatLong` | `(lat, long)` | Height at coordinates. |
| `Nav.getColorAtLatLong` | `(lat, long)` | Surface color at coordinates. |
| `Nav.castRay` | `(origin, dir)` | Raycast. |

### `Velocity`
| Function | Signature | Description |
|----------|-----------|-------------|
| `Velocity.orbit` | `()` | Orbital velocity. |
| `Velocity.surface` | `()` | Surface velocity. |
| `Velocity.target` | `()` | Target-relative velocity. |
| `Velocity.gravity` | `()` | Gravitational acceleration. |
| `Velocity.drag` | `()` | Drag acceleration. |
| `Velocity.acceleration` | `()` | Total acceleration. |
| `Velocity.angular` | `()` | Angular velocity. |
| `Velocity.lateral` | `()` | Lateral velocity. |
| `Velocity.vertical` | `()` | Vertical velocity. |
| `Velocity.mach` | `()` | Mach number. |

### `Input` (expressions)
| Function | Signature | Description |
|----------|-----------|-------------|
| `Input.throttle` | `()` | Current throttle. |
| `Input.pitch` | `()` | Current pitch input. |
| `Input.roll` | `()` | Current roll input. |
| `Input.yaw` | `()` | Current yaw input. |
| `Input.brake` | `()` | Current brake. |
| `Input.slider` | `(index)` | Slider value by index. |
| `Input.translateForward` | `()` | Forward translation input. |
| `Input.translateRight` | `()` | Right translation input. |
| `Input.translateUp` | `()` | Up translation input. |
| `Input.translationMode` | `()` | Current translation mode. |
| `Input.activationGroup` | `(group)` | Activation group state. |

### `Craft` (expressions)
| Function | Signature | Description |
|----------|-----------|-------------|
| `Craft.name` | `()` or `(id)` | Craft name or part name by id. |
| `Craft.idOf` | `(name)` | ID of a named craft/part. |
| `Craft.currentStage` | `()` | Current stage number. |
| `Craft.numStages` | `()` | Total stage count. |
| `Craft.isGrounded` | `()` or `(partId)` | Whether craft/part is grounded. |
| `Craft.solarRadiation` | `()` | Solar radiation level. |
| `Craft.pitchPID` | `()` | Pitch PID values. |
| `Craft.rollPID` | `()` | Roll PID values. |
| `Craft.targetName` | `()` | Target name. |
| `Craft.targetPlanet` | `()` | Target planet. |
| `Craft.position` | `(id)` | Position of craft/part. |
| `Craft.velocity` | `(id)` | Velocity of craft/part. |
| `Craft.isPlayer` | `(id)` | Whether craft is player-controlled. |
| `Craft.altitude` | `(id)` | Altitude of craft/part. |
| `Craft.isDestroyed` | `(id)` | Whether craft/part is destroyed. |
| `Craft.mass` | `(id)` | Mass of craft/part. |
| `Craft.partCount` | `(id)` | Number of parts. |
| `Craft.planet` | `(id)` | Planet of craft/part. |
| `Craft.boundingBoxMin` | `(id)` | Bounding box minimum. |
| `Craft.boundingBoxMax` | `(id)` | Bounding box maximum. |
| `Craft.orbitAp` | `(id)` | Apoapsis of craft/part. |
| `Craft.orbitPe` | `(id)` | Periapsis of craft/part. |
| `Craft.orbitPeriod` | `(id)` | Orbital period of craft/part. |

### `Camera` (expressions)
| Function | Signature | Description |
|----------|-----------|-------------|
| `Camera.position` | `()` | Camera position. |
| `Camera.pointing` | `()` | Camera pointing direction. |
| `Camera.upDirection` | `()` | Camera up direction. |

### `Time` (expressions)
| Function | Signature | Description |
|----------|-----------|-------------|
| `Time.sinceLaunch` | `()` | Time since launch. |
| `Time.delta` | `()` | Delta time. |
| `Time.total` | `()` | Total mission time. |
| `Time.warpAmount` | `()` | Current warp factor. |
| `Time.realTime` | `()` | Real-world time. |

### `Planet` (expressions)
| Function | Signature | Description |
|----------|-----------|-------------|
| `Planet.radius` | `(id)` | Planet radius. |
| `Planet.mass` | `(id)` | Planet mass. |
| `Planet.solidGround` | `(id)` | Whether planet has solid ground. |
| `Planet.surfaceAirDensity` | `(id)` | Surface air density. |
| `Planet.atmosphereHeight` | `(id)` | Atmosphere height. |
| `Planet.atmosphereScaleHeight` | `(id)` | Atmosphere scale height. |
| `Planet.atmosphereElements` | `(id)` | Atmospheric elements. |
| `Planet.atmosphereComposition` | `(id)` | Atmospheric composition. |
| `Planet.soiRadius` | `(id)` | Sphere of influence radius. |
| `Planet.solarPosition` | `(id)` | Solar position relative to planet. |
| `Planet.childPlanets` | `(id)` | Child planets/moons. |
| `Planet.crafts` | `(id)` | Crafts at planet. |
| `Planet.craftID` | `(id)` | Craft ID at planet. |
| `Planet.parent` | `(id)` | Parent body. |
| `Planet.structures` | `(id)` | Structures on planet. |
| `Planet.hydrosphereElement` | `(id)` | Hydrosphere element. |
| `Planet.hydrosphereComposition` | `(id)` | Hydrosphere composition. |
| `Planet.hydrosphereDensity` | `(id)` | Hydrosphere density. |
| `Planet.dayLength` | `(id)` | Day length. |
| `Planet.yearLength` | `(id)` | Year length. |
| `Planet.velocity` | `(id)` | Planet velocity. |
| `Planet.orbitAp` | `(id)` | Apoapsis around parent. |
| `Planet.orbitPe` | `(id)` | Periapsis around parent. |
| `Planet.orbitPeriod` | `(id)` | Orbital period around parent. |
| `Planet.orbitApTime` | `(id)` | Time to apoapsis. |
| `Planet.orbitPeTime` | `(id)` | Time to periapsis. |
| `Planet.orbitInclination` | `(id)` | Orbital inclination. |
| `Planet.orbitEccentricity` | `(id)` | Orbital eccentricity. |
| `Planet.orbitMeanAnomaly` | `(id)` | Mean anomaly. |
| `Planet.orbitMeanMotion` | `(id)` | Mean motion. |
| `Planet.orbitPeriapsisArgument` | `(id)` | Argument of periapsis. |
| `Planet.orbitRightAscension` | `(id)` | Right ascension of ascending node. |
| `Planet.orbitTrueAnomaly` | `(id)` | True anomaly. |
| `Planet.orbitSemiMajorAxis` | `(id)` | Semi-major axis. |
| `Planet.orbitSemiMinorAxis` | `(id)` | Semi-minor axis. |

### `Part` (expressions)
| Function | Signature | Description |
|----------|-----------|-------------|
| `Part.idOf` | `(name)` | ID of a named part. |
| `Part.activated` | `(id)` | Whether part is activated. |
| `Part.name` | `(id)` | Part name. |
| `Part.mass` | `(id)` | Part mass. |
| `Part.dryMass` | `(id)` | Part dry mass. |
| `Part.wetMass` | `(id)` | Part wet mass. |
| `Part.partType` | `(id)` | Part type. |
| `Part.position` | `(id)` | Part position. |
| `Part.temperature` | `(id)` | Part temperature. |
| `Part.drag` | `(id)` | Part drag. |
| `Part.programPartId` | `(id)` | Part running the program. |
| `Part.minPartId` | `(id)` | Minimum part ID. |
| `Part.maxPartId` | `(id)` | Maximum part ID. |
| `Part.underwater` | `(id)` | Whether part is underwater. |
| `Part.localToPCI` | `(local, ref)` | Convert local to PCI coordinates. |
| `Part.pciToLocal` | `(pci, ref)` | Convert PCI to local coordinates. |
