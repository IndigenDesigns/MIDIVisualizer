#include "data.h"
const std::map<std::string, std::string> shaders = {
{ "background_vert", "#version 330\n layout(location = 0) in vec3 v;\n out INTERFACE {\n 	vec2 uv;\n } Out ;\n void main(){\n 	\n 	// We directly output the position.\n 	gl_Position = vec4(v, 1.0);\n 	// Output the UV coordinates computed from the positions.\n 	Out.uv = v.xy * 0.5 + 0.5;\n 	\n }\n "}, 
{ "background_frag", "#version 330\n in INTERFACE {\n 	vec2 uv;\n } In ;\n uniform float time;\n uniform float secondsPerMeasure;\n uniform vec2 inverseScreenSize;\n uniform bool useDigits = true;\n uniform bool useHLines = true;\n uniform bool useVLines = true;\n uniform float minorsWidth = 1.0;\n uniform sampler2D screenTexture;\n uniform vec3 textColor = vec3(1.0);\n uniform vec3 linesColor = vec3(1.0);\n uniform vec3 keysColor = vec3(0.0);\n const bool isMinor[88] = bool[](true, false, true, true, false, true, true, true, false, true, true, false, true, true, true, false, true, true, false, true, true, true, false, true, true, false, true, true, true, false, true, true, false, true, true, true, false, true, true, false, true, true, true, false, true, true, false, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false);\n const float octaveLinesPositions[8] = float[](2.0/52.0, 9.0/52.0, 16.0/52.0, 23.0/52.0, 30.0/52.0, 37.0/52.0, 44.0/52.0, 51.0/52.0);\n 			\n uniform float mainSpeed;\n #define bottomLimit 0.25\n out vec3 fragColor;\n float printDigit(int digit, vec2 uv){\n 	// Clamping to avoid artifacts.\n 	if(uv.x < 0.01 || uv.x > 0.99 || uv.y < 0.01 || uv.y > 0.99){\n 		return 0.0;\n 	}\n 	\n 	// UV from [0,1] to local tile frame.\n 	vec2 localUV = uv * vec2(50.0/256.0,0.5);\n 	// Select the digit.\n 	vec2 globalUV = vec2( mod(digit,5)*50.0/256.0,digit < 5 ? 0.5 : 0.0);\n 	// Combine global and local shifts.\n 	vec2 finalUV = globalUV + localUV;\n 	\n 	// Read from font atlas. Return if above a threshold.\n 	float isIn = texture(screenTexture, finalUV).r;\n 	return isIn < 0.5 ? 0.0 : isIn ;\n 	\n }\n float printNumber(float num, vec2 position, vec2 uv, vec2 scale){\n 	if(num < -0.1){\n 		return 0.0f;\n 	}\n 	if(position.y > 1.0 || position.y < 0.0){\n 		return 0.0;\n 	}\n 	\n 	// We limit to the [0,999] range.\n 	float number = min(999.0, max(0.0,num));\n 	\n 	// Extract digits.\n 	int hundredDigit = int(floor( number / 100.0 ));\n 	int tenDigit	 = int(floor( number / 10.0 - hundredDigit * 10.0));\n 	int unitDigit	 = int(floor( number - hundredDigit * 100.0 - tenDigit * 10.0));\n 	\n 	// Position of the text.\n 	vec2 initialPos = scale*(uv-position);\n 	\n 	// Get intensity for each digit at the current fragment.\n 	float hundred = printDigit(hundredDigit, initialPos);\n 	float ten	  =	printDigit(tenDigit,	 initialPos - vec2(scale.x * 0.009,0.0));\n 	float unit	  = printDigit(unitDigit,	 initialPos - vec2(scale.x * 0.009 * 2.0,0.0));\n 	\n 	// If hundred digit == 0, hide it.\n 	float hundredVisibility = (1.0-step(float(hundredDigit),0.5));\n 	hundred *= hundredVisibility;\n 	// If ten digit == 0 and hundred digit == 0, hide ten.\n 	float tenVisibility = max(hundredVisibility,(1.0-step(float(tenDigit),0.5)));\n 	ten*= tenVisibility;\n 	\n 	return hundred + ten + unit;\n }\n void main(){\n 	\n 	vec3 bgColor = vec3(0.0);\n 	// Octaves lines.\n 	for(int i = 0; i < 8; i++){\n 		float lineIntensity = useVLines ? (0.7 * step(abs(In.uv.x - octaveLinesPositions[i]),inverseScreenSize.x)) : 0.0;\n 		bgColor = mix(bgColor, linesColor, lineIntensity);\n 	}\n 	\n 	vec2 scale = 1.5*vec2(64.0,50.0*inverseScreenSize.x/inverseScreenSize.y);\n 	\n 	// Text on the side.\n 	int currentMesure = int(floor(time/secondsPerMeasure));\n 	// How many mesures do we check.\n 	int count = int(ceil(0.75*(2.0/mainSpeed)))+2;\n 	\n 	for(int i = 0; i < count; i++){\n 		// Compute position of the measure currentMesure+i.\n 		vec2 position = vec2(0.005,bottomLimit + (secondsPerMeasure*(currentMesure+i) - time)*mainSpeed*0.5);\n 		\n 		// Compute color for the number display, and for the horizontal line.\n 		float numberIntensity = useDigits ? printNumber(currentMesure + i,position, In.uv, scale) : 0.0;\n 		bgColor = mix(bgColor, textColor, numberIntensity);\n 		float lineIntensity = useHLines ? (0.25*(step(abs(In.uv.y - position.y - 0.5 / scale.y), inverseScreenSize.y))) : 0.0;\n 		bgColor = mix(bgColor, linesColor, lineIntensity);\n 	}\n 	\n 	if(all(equal(bgColor, vec3(0.0)))){\n 		// Transparent background.\n 		discard;\n 	}\n 	\n 	fragColor = bgColor;\n 	\n }\n "},
{ "flashes_vert", "#version 330\n layout(location = 0) in vec2 v;\n layout(location = 1) in int isOn;\n uniform float time;\n uniform vec2 inverseScreenSize;\n uniform float userScale = 1.0;\n #define notesCount 52.0\n const float shifts[88] = float[](0, 0.5, 1, 2, 2.5, 3, 3.5, 4, 5, 5.5, 6, 6.5, 7, 7.5, 8, 9, 9.5, 10, 10.5, 11, 12, 12.5, 13, 13.5, 14, 14.5, 15, 16, 16.5, 17, 17.5, 18, 19, 19.5, 20, 20.5, 21, 21.5, 22, 23, 23.5, 24, 24.5, 25, 26, 26.5, 27, 27.5, 28, 28.5, 29, 30, 30.5, 31, 31.5, 32, 33, 33.5, 34, 34.5, 35, 35.5, 36, 37, 37.5, 38, 38.5, 39, 40, 40.5, 41, 41.5, 42, 42.5, 43, 44, 44.5, 45, 45.5, 46, 47, 47.5, 48, 48.5, 49, 49.5, 50, 51);\n const vec2 scale = 0.9*vec2(3.5,3.0);\n out INTERFACE {\n 	vec2 uv;\n 	float on;\n 	float id;\n } Out;\n void main(){\n 	\n 	// Scale quad, keep the square ratio.\n 	vec2 scaledPosition = v * 2.0 * scale * userScale/notesCount * vec2(1.0, inverseScreenSize.y/inverseScreenSize.x);\n 	// Shift based on note/flash id.\n 	vec2 globalShift = vec2(-1.0 + (shifts[gl_InstanceID] * 2.0 + 1.0) / notesCount,-0.5);\n 	\n 	gl_Position = vec4(scaledPosition + globalShift, 0.0 , 1.0) ;\n 	\n 	// Pass infos to the fragment shader.\n 	Out.uv = v;\n 	Out.on = float(isOn>0);\n 	Out.id = float(gl_InstanceID);\n 	\n }\n "}, 
{ "flashes_frag", "#version 330\n in INTERFACE {\n 	vec2 uv;\n 	float on;\n 	float id;\n } In;\n uniform sampler2D textureFlash;\n uniform float time;\n uniform vec3 baseColor;\n #define numberSprites 8.0\n out vec4 fragColor;\n float rand(vec2 co){\n 	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);\n }\n void main(){\n 	\n 	// If not on, discard flash immediatly.\n 	if(In.on < 0.5){\n 		discard;\n 	}\n 	\n 	float mask = 0.0;\n 	\n 	// If up half, read from texture atlas.\n 	if(In.uv.y > 0.0){\n 		// Select a sprite, depending on time and flash id.\n 		float shift = floor(mod(15.0 * time, numberSprites)) + floor(rand(In.id * vec2(time,1.0)));\n 		vec2 globalUV = vec2(0.5 * mod(shift, 2.0), 0.25 * floor(shift/2.0));\n 		\n 		// Scale UV to fit in one sprite from atlas.\n 		vec2 localUV = In.uv * 0.5 + vec2(0.25,-0.25);\n 		localUV.y = min(-0.05,localUV.y); //Safety clamp on the upper side (or you could set clamp_t)\n 		\n 		// Read in black and white texture do determine opacity (mask).\n 		vec2 finalUV = globalUV + localUV;\n 		mask = texture(textureFlash,finalUV).r;\n 	}\n 	\n 	// Colored sprite.\n 	vec4 spriteColor = vec4(baseColor,In.on * mask);\n 	\n 	// Circular halo effect.\n 	float haloAlpha = 1.0 - smoothstep(0.07,0.5,length(In.uv));\n 	vec4 haloColor = vec4(1.0,1.0,1.0, In.on * haloAlpha * 0.92);\n 	\n 	// Mix the sprite color and the halo effect.\n 	fragColor = mix(spriteColor, haloColor, haloColor.a);\n 	\n 	// Boost intensity.\n 	fragColor *= 1.1;\n 	\n }\n "},
{ "notes_vert", "#version 330\n layout(location = 0) in vec2 v;\n layout(location = 1) in vec4 id; //note id, start, duration, is minor\n uniform float time;\n uniform float mainSpeed;\n uniform float minorsWidth = 1.0;\n #define notesCount 52.0\n out INTERFACE {\n 	float isMinor;\n 	vec2 uv;\n 	vec2 noteSize;\n } Out;\n void main(){\n 	\n 	float scalingFactor = id.w != 0.0 ? minorsWidth : 1.0;\n 	// Size of the note : width, height based on duration and current speed.\n 	Out.noteSize = vec2(0.9*2.0/notesCount * scalingFactor, id.z*mainSpeed);\n 	\n 	// Compute note shift.\n 	// Horizontal shift based on note id, width of keyboard, and if the note is minor or not.\n 	// Vertical shift based on note start time, current time, speed, and height of the note quad.\n 	const float a = (1.0/(notesCount-1.0)) * (2.0 - 2.0/notesCount);\n 	const float b = -1.0 + 1.0/notesCount;\n 	vec2 noteShift = vec2(id.x * a + b + id.w/notesCount, Out.noteSize.y * 0.5 - 0.5 + mainSpeed * (id.y - time));\n 	\n 	// Scale uv.\n 	Out.uv = Out.noteSize * v;\n 	Out.isMinor = id.w;\n 	// Output position.\n 	gl_Position = vec4(Out.noteSize * v + noteShift, 0.0 , 1.0) ;\n 	\n }\n "}, 
{ "notes_frag", "#version 330\n in INTERFACE {\n 	float isMinor;\n 	vec2 uv;\n 	vec2 noteSize;\n } In;\n uniform vec3 baseColor;\n uniform vec3 minorColor;\n uniform vec2 inverseScreenSize;\n #define cornerRadius 0.01\n #define bottomLimit 0.25\n out vec3 fragColor;\n void main(){\n 	\n 	// If lower area of the screen, discard fragment as it should be hidden behind the keyboard.\n 	if(gl_FragCoord.y < bottomLimit/inverseScreenSize.y){\n 		discard;\n 	}\n 	\n 	// Rounded corner (super-ellipse equation).\n 	float radiusPosition = pow(abs(In.uv.x/(0.5*In.noteSize.x)), In.noteSize.x/cornerRadius) + pow(abs(In.uv.y/(0.5*In.noteSize.y)), In.noteSize.y/cornerRadius);\n 	\n 	if(	radiusPosition > 1.0){\n 		discard;\n 	}\n 	\n 	// Fragment color.\n 	fragColor = mix(baseColor, minorColor, In.isMinor);\n 	\n 	if(	radiusPosition > 0.8){\n 		fragColor *= 1.05;\n 	}\n 	\n }\n "},
{ "particles_vert", "#version 330\n layout(location = 0) in vec2 v;\n uniform float time;\n uniform float scale;\n uniform vec3 baseColor;\n uniform vec2 inverseScreenSize;\n uniform sampler2D textureParticles;\n uniform vec2 inverseTextureSize;\n uniform int globalId;\n uniform float duration;\n uniform int texCount;\n uniform float expansionFactor = 1.0;\n uniform float speedScaling = 0.2;\n #define notesCount 52.0\n const float shifts[88] = float[](0, 0.5, 1, 2, 2.5, 3, 3.5, 4, 5, 5.5, 6, 6.5, 7, 7.5, 8, 9, 9.5, 10, 10.5, 11, 12, 12.5, 13, 13.5, 14, 14.5, 15, 16, 16.5, 17, 17.5, 18, 19, 19.5, 20, 20.5, 21, 21.5, 22, 23, 23.5, 24, 24.5, 25, 26, 26.5, 27, 27.5, 28, 28.5, 29, 30, 30.5, 31, 31.5, 32, 33, 33.5, 34, 34.5, 35, 35.5, 36, 37, 37.5, 38, 38.5, 39, 40, 40.5, 41, 41.5, 42, 42.5, 43, 44, 44.5, 45, 45.5, 46, 47, 47.5, 48, 48.5, 49, 49.5, 50, 51);\n out INTERFACE {\n 	vec4 color;\n 	vec2 uv;\n 	float id;\n } Out;\n float rand(vec2 co){\n 	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);\n }\n void main(){\n 	Out.id = float(gl_InstanceID % texCount);\n 	Out.uv = v + 0.5;\n 	// Fade color based on time.\n 	Out.color = vec4(baseColor, 1.0-time*time);\n 	\n 	float localTime = speedScaling * time * duration;\n 	float particlesCount = 1.0/inverseTextureSize.y;\n 	\n 	// Pick particle id at random.\n 	float particleId = float(gl_InstanceID) + floor(particlesCount * 10.0 * rand(vec2(globalId,globalId)));\n 	float textureId = mod(particleId,particlesCount);\n 	float particleShift = floor(particleId/particlesCount);\n 	\n 	// Particle uv, in pixels.\n 	vec2 particleUV = vec2(localTime / inverseTextureSize.x + 10.0 * particleShift, textureId);\n 	// UV in [0,1]\n 	particleUV = (particleUV+0.5)*vec2(1.0,-1.0)*inverseTextureSize;\n 	// Avoid wrapping.\n 	particleUV.x = clamp(particleUV.x,0.0,1.0);\n 	// We want to skip reading from the very beginning of the trajectories because they are identical.\n 	// particleUV.x = 0.95 * particleUV.x + 0.05;\n 	// Read corresponding trajectory to get particle current position.\n 	vec3 position = texture(textureParticles, particleUV).xyz;\n 	// Center position (from [0,1] to [-0.5,0.5] on x axis.\n 	position.x -= 0.5;\n 	\n 	// Compute shift, randomly disturb it.\n 	vec2 shift = 0.5*position.xy;\n 	float random = rand(vec2(particleId + float(globalId),time*0.000002+100.0*float(globalId)));\n 	shift += vec2(0.0,0.1*random);\n 	\n 	// Scale shift with time (expansion effect).\n 	shift = shift*time*expansionFactor;\n 	// and with altitude of the particle (ditto).\n 	shift.x *= max(0.5, pow(shift.y,0.3));\n 	\n 	// Combine global shift (due to note id) and local shift (based on read position).\n 	vec2 globalShift = vec2(-1.0 + (shifts[globalId] * 2.0 + 1.0) / notesCount,-0.52);\n 	vec2 localShift = 0.003 * scale * v + shift * duration * vec2(1.0,0.5);\n 	vec2 screenScaling = vec2(1.0,inverseScreenSize.y/inverseScreenSize.x);\n 	vec2 finalPos = globalShift + screenScaling * localShift;\n 	\n 	// Discard particles that reached the end of their trajectories by putting them off-screen.\n 	finalPos = mix(vec2(-200.0),finalPos, position.z);\n 	// Output final particle position.\n 	gl_Position = vec4(finalPos,0.0,1.0);\n 	\n 	\n }\n "}, 
{ "particles_frag", "#version 330\n in INTERFACE {\n 	vec4 color;\n 	vec2 uv;\n 	float id;\n } In;\n uniform sampler2DArray lookParticles;\n out vec4 fragColor;\n void main(){\n 	float alpha = texture(lookParticles, vec3(In.uv, In.id)).r;\n 	fragColor = In.color;\n 	fragColor.a *= alpha;\n }\n "},
{ "particlesblur_vert", "#version 330\n layout(location = 0) in vec3 v;\n out INTERFACE {\n 	vec2 uv;\n } Out ;\n void main(){\n 	\n 	// We directly output the position.\n 	gl_Position = vec4(v, 1.0);\n 	// Output the UV coordinates computed from the positions.\n 	Out.uv = v.xy * 0.5 + 0.5;\n 	\n }\n "}, 
{ "particlesblur_frag", "#version 330\n in INTERFACE {\n 	vec2 uv;\n } In ;\n uniform sampler2D screenTexture;\n uniform vec2 inverseScreenSize;\n uniform vec3 backgroundColor = vec3(0.0, 0.0, 0.0);\n uniform float attenuationFactor = 0.99;\n out vec3 fragColor;\n void main(){\n 	\n 	// We have to unroll the box blur loop manually.\n 	// 5x5 blur, using a sparse sample grid.\n 	vec3 color = texture(screenTexture, In.uv).rgb;\n 	\n 	color += textureOffset(screenTexture, In.uv, 2*ivec2(-2,-2)).rgb;\n 	color += textureOffset(screenTexture, In.uv, 2*ivec2(-2,2)).rgb;\n 	color += textureOffset(screenTexture, In.uv, 2*ivec2(-1,0)).rgb;\n 	color += textureOffset(screenTexture, In.uv, 2*ivec2(0,-1)).rgb;\n 	color += textureOffset(screenTexture, In.uv, 2*ivec2(0,1)).rgb;\n 	color += textureOffset(screenTexture, In.uv, 2*ivec2(1,0)).rgb;\n 	color += textureOffset(screenTexture, In.uv, 2*ivec2(2,-2)).rgb;\n 	color += textureOffset(screenTexture, In.uv, 2*ivec2(2,2)).rgb;\n 	\n 	// Include decay for fade out.\n 	fragColor = mix(backgroundColor, color/9.0, attenuationFactor);\n 	\n }\n "},
{ "screenquad_vert", "#version 330\n layout(location = 0) in vec3 v;\n out INTERFACE {\n 	vec2 uv;\n } Out ;\n void main(){\n 	\n 	// We directly output the position.\n 	gl_Position = vec4(v, 1.0);\n 	// Output the UV coordinates computed from the positions.\n 	Out.uv = v.xy * 0.5 + 0.5;\n 	\n }\n "}, 
{ "screenquad_frag", "#version 330\n in INTERFACE {\n 	vec2 uv;\n } In ;\n uniform sampler2D screenTexture;\n uniform vec2 inverseScreenSize;\n out vec3 fragColor;\n void main(){\n 	\n 	fragColor = texture(screenTexture,In.uv).rgb;\n 	\n }\n "},
{ "keys_vert", "#version 330\n layout(location = 0) in vec2 v;\n out INTERFACE {\n 	vec2 uv;\n } Out ;\n void main(){\n 	// Input are in -0.5,0.5\n 	// We directly output the position.\n 	float yShift = 0.5*(v.y + 0.5)-1.0; \n 	gl_Position = vec4(v.x*2.0, yShift, 0.0, 1.0);\n 	// Output the UV coordinates computed from the positions.\n 	Out.uv = v.xy + 0.5;\n 	\n }\n "}, 
{ "keys_frag", "#version 330\n in INTERFACE {\n 	vec2 uv;\n } In ;\n uniform vec2 inverseScreenSize;\n uniform float minorsWidth = 1.0;\n uniform vec3 keysColor = vec3(0.0);\n const bool isMinor[88] = bool[](true, false, true, true, false, true, true, true, false, true, true, false, true, true, true, false, true, true, false, true, true, true, false, true, true, false, true, true, true, false, true, true, false, true, true, true, false, true, true, false, true, true, true, false, true, true, false, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false);\n const float octaveLinesPositions[8] = float[](2.0/52.0, 9.0/52.0, 16.0/52.0, 23.0/52.0, 30.0/52.0, 37.0/52.0, 44.0/52.0, 51.0/52.0);\n 			\n out vec3 fragColor;\n void main(){\n 	\n 	// White keys, and separators.\n 	float intensity = int(abs(fract(In.uv.x*52.0)) >= 2.0 * 52.0 * inverseScreenSize.x);\n 		\n 	// Upper keyboard.\n 	if(gl_FragCoord.y > 0.10/inverseScreenSize.y){\n 		// Handle black keys.\n 		int index = int(floor(In.uv.x*52.0+0.5))-1;\n 	\n 		if(index > 0 && isMinor[index]){\n 			// If the minor keys are not thinner, preserve a 1 px margin on each side.\n 			float marginSize = minorsWidth != 1.0 ? minorsWidth : 1.0 - (2.0*52.0*inverseScreenSize.x);\n 			intensity = step(marginSize, abs(fract(In.uv.x*52.0+0.5)*2.0-1.0));\n 		}\n 	}\n 	fragColor = mix(keysColor, vec3(1.0), intensity);\n 	\n }\n "}
};
