#shader vertex
#version 330 core

layout(location = 0) in vec4 vertex;
out vec2 texCoords;

uniform mat4 projection;
uniform vec2 size;
uniform vec2 scale;
uniform bool flipVertically;

void main() {
	gl_Position = projection * vec4(vertex.x * size.x * scale.x, vertex.y * size.y * scale.y, 0.0f, 1.0f);

	if (flipVertically) {
		texCoords = vec2(vertex.z, -vertex.w);
	} else {
		texCoords = vec2(vertex.z, vertex.w);
	}
}

#shader fragment
#version 330 core

in vec2 texCoords;
out vec4 fragColor;

uniform sampler2D image;

uniform float time;
uniform vec2 position;

uniform bool useTonemapping;
uniform bool adjustment_NoTonemapping;
uniform bool adjustment_UseFlatTonemapping;
uniform bool adjustment_ShowAlphaCheckerboard;
uniform bool adjustment_ShowZebraPattern;

uniform bool adjustment_Grayscale;
uniform bool adjustment_Invert;
uniform float adjustment_ZebraPatternThreshold;
uniform float adjustment_Exposure;
uniform float adjustment_Offset;
uniform vec4 adjustment_ChannelMultiplier;

float checkerboardSize = 8.0f;

const mat3 ACESInputMat = mat3(
	0.59719f, 0.35458f, 0.04823f,
	0.07600f, 0.90834f, 0.015566f,
	0.02840f, 0.13383f, 0.83777f
);

const mat3 ACESOutputMat = mat3(
	1.60475f, -0.53108f, -0.07367f,
   -0.10208f,  1.10813f, -0.00605f,
   -0.00327f, -0.07276f,  1.07602f
);

const mat3 HighlightFixMat = mat3( // Prevents bright blues going to purple
	0.9404372683, 0.0183068787, 0.0778696104,
	0.0083786969, 0.8286599939, 0.1629613092,
	0.0005471261, 0.0008833746, 1.0003362486
);

void main() {
	vec4 sampled = texture(image, texCoords).rgba;

	vec2 fragCoord = gl_FragCoord.xy;
	vec2 pos = floor((fragCoord + vec2(-position.x, position.y)) / checkerboardSize);
	float patternMask = mod(pos.x + mod(pos.y, 2.0f), 2.0f) / 3.0f + 0.66f;
	vec4 checkerboard = vec4(patternMask, patternMask, patternMask, 1.0f);

	float zebraPos = floor(((fragCoord.x + fragCoord.y) + (time * 30.0f)) / 3.0f);
	float zebra = mod(zebraPos, 3.0f) / 3.0f + 0.66f;

	fragColor = sampled;

	// Invert image
	if (adjustment_Invert) {
		fragColor = vec4(1.0f - sampled.r, 1.0f - sampled.g, 1.0f - sampled.b, sampled.a);
	}

	// Exposure
	fragColor = vec4(fragColor.rgb * pow(2.0f, adjustment_Exposure), fragColor.a);

	// Tonemapping
	if (useTonemapping && !adjustment_NoTonemapping) {
		//fragColor.rgb = fragColor.rgb * 1.6f; // Should multiply because ACES darkens stuff a bit
		fragColor = vec4(fragColor.rgb * ACESInputMat, fragColor.a);

		// Flat tonemapping or not
		if (adjustment_UseFlatTonemapping) {
			fragColor.r = sqrt(fragColor.r / (fragColor.r + 1.0f));
			fragColor.g = sqrt(fragColor.g / (fragColor.g + 1.0f));
			fragColor.b = sqrt(fragColor.b / (fragColor.b + 1.0f));
		} else {
			vec3 c = fragColor.rgb;

			vec3 a = c * (c + 0.0245786f) - 0.000090537f;
			vec3 b = c * ((c * 0.983729f) + 0.4329510f) + 0.238081f;
			c = a / b;

			fragColor = vec4(c, fragColor.a);
		}

		fragColor = vec4(fragColor.rgb * ACESOutputMat, fragColor.a);
	}

	// Offset
	fragColor += vec4(adjustment_Offset, adjustment_Offset, adjustment_Offset, 0.0f);

	// Channel multiplier
	fragColor = fragColor * adjustment_ChannelMultiplier;

	// Grayscale
	if (adjustment_Grayscale) {
		float average = (fragColor.r + fragColor.g + fragColor.b) / 3.0f;
		fragColor = vec4(average, average, average, fragColor.a);
	}

	// Alpha checkerboard
	if (adjustment_ShowAlphaCheckerboard) {
		fragColor = mix(checkerboard, fragColor, fragColor.a);
	}

	// Zebra pattern
	{
		float t = adjustment_ZebraPatternThreshold;

		if (adjustment_ShowZebraPattern && fragColor.r * fragColor.a >= t && fragColor.g * fragColor.a >= t && fragColor.b * fragColor.a >= t) {
			fragColor = vec4(zebra);
		}
	}
}