uniform sampler2D 	source;
uniform vec2 		offsetFactor;

void main()
{
	vec2 textureCoordinates = gl_TexCoord[0].xy;
	vec4 color = vec4(0.0);
    color.a = texture2D(source, textureCoordinates).a;

	color += texture2D(source, textureCoordinates - 3.0 * offsetFactor) * 0.000440745;
	color += texture2D(source, textureCoordinates - 2.0 * offsetFactor) * 0.0219104;
	color += texture2D(source, textureCoordinates - offsetFactor) * 0.228311;
	color += texture2D(source, textureCoordinates) * 0.498678;
	color += texture2D(source, textureCoordinates + offsetFactor) * 0.228311;
	color += texture2D(source, textureCoordinates + 2.0 * offsetFactor) * 0.0219104;
	color += texture2D(source, textureCoordinates + 3.0 * offsetFactor) * 0.000440745;

	gl_FragColor = color;
}