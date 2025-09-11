//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#ifndef OT_TONE_MAPPING_GLSL
#define OT_TONE_MAPPING_GLSL


vec3 tonemapReinhardSimple(vec3 color) {
	// Photographic Tone Reproduction For Digital Images" by Reinhard et al. 2002
	// https://www.researchgate.net/publication/2908938_Photographic_Tone_Reproduction_For_Digital_Images
	return color / (color + vec3(1.0f));
}

vec3 tonemapReinhardExtended(vec3 color) {
	// Photographic Tone Reproduction For Digital Images" by Reinhard et al. 2002
	// https://www.researchgate.net/publication/2908938_Photographic_Tone_Reproduction_For_Digital_Images
	return (color * (vec3(1.0f) + (color / (4.0f * 4.0f))) / (vec3(1.0f) + color));
}

vec3 tonemapFilmic(vec3 color) {
	// Filmic Tonemapping Operators
	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	color = max(vec3(0.0f), color - 0.004f);
	color = (color * (6.2f * color + 0.5f)) / (color * (6.2f * color + 1.7f) + 0.06f);
	return pow(color, vec3(2.2f));
}

vec3 tonemapAcesFilmic(vec3 color) {
	// ACES Filmic Tone Mapping Curve, Krzysztof Narkowicz 2015
	// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
	return clamp((color * (2.51f * color + 0.03f)) / (color * (2.43f * color + 0.59f) + 0.14f),0.0f, 1.0f);
}

vec3 tonemapUncharted2(vec3 color) {
	// Uncharted 2 filmic operator, John Hable 2010
	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	// values used are slightly modified, comments have the original values
	float whiteScale = 0.72513f;
	float ExposureBias = 2.0f;

	float A = 0.22f; // shoulder strength // 0.15
	float B = 0.30f; // linear strength // 0.50
	float C = 0.10f; // linear angle
	float D = 0.20f; // toe strength
	float E = 0.01f; // toe numerator // 0.02
	float F = 0.30f; // toe denominator

	color *= ExposureBias;
	color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
	return color / whiteScale;
}

vec3 tonemapLottes(vec3 color) {
	// Advanced Techniques and Optimization of HDR Color Pipelines, Timothy Lottes 2016
	// https://gpuopen.com/wp-content/uploads/2016/03/GdcVdrLottes.pdf
	vec3 a = vec3(1.6f);
	vec3 d = vec3(0.977f);
	vec3 hdrMax = vec3(8.0f);
	vec3 midIn = vec3(0.18f);
	vec3 midOut = vec3(0.267f);

	vec3 b = (-pow(midIn, a) + pow(hdrMax, a) * midOut) /
		((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);

	vec3 c = (pow(hdrMax, a * d) * pow(midIn, a) - pow(hdrMax, a) * pow(midIn, a * d) * midOut) /
		((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);

  return pow(color, a) / (pow(color, a * d) * b + c);
}

vec3 tonemapUchimura(vec3 color) {
	// HDR theory and practice, Hajime Uchimura, 2017
	// https://www.desmos.com/calculator/gslcdxvipg
	float P = 1.0f;  // max display brightness
	float a = 1.0f;  // contrast
	float m = 0.22f; // linear section start
	float l = 0.4f;  // linear section length
	float c = 1.33f; // black
	float b = 0.0f;  // pedestal

	float l0 = ((P - m) * l) / a;
	float L0 = m - m / a;
	float L1 = m + (1.0 - m) / a;
	float S0 = m + l0;
	float S1 = m + a * l0;
	float C2 = (a * P) / (P - S1);
	float CP = -C2 / P;

	vec3 w0 = 1.0f - smoothstep(0.0f, m, color);
	vec3 w2 = step(m + l0, color);
	vec3 w1 = 1.0f - w0 - w2;

	vec3 T = m * pow(color / m, vec3(c)) + b;
	vec3 S = P - (P - S1) * exp(CP * (color - S0));
	vec3 L = m + a * (color - m);

	return T * w0 + L * w1 + S * w2;
}

#endif