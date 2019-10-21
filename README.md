I implemented:

	Texture rendering via 1D, 2D, 3D, and cube map textures.

	User-defined, linear, cylindrical, spherical, normal, and reflection-based
		texture coordinate generation.

		'User-defined' accepts texture coordinates specified by the user via
			the osuTexCoord() function.

		'Linear' copies the surface point into the texture coordinate.

		'Cylindrical' maps the yaw, the height, and the radial distance of each
			surface point into the texture coordinates.

		'Spherical' maps the yaw, the pitch, and the distance of each surface
			point into the texture coordinates.

		'Normal' maps the surface normal vector at each vertex, specified by
			the user via the osuNormal() function, into the texture coordinates.
			This method is denoted "Cel" texture coordinate generation within
				the demonstration application.

		'Reflection' computes the vector from each vertex to the viewer position,
			reflects it about the plane perpendicular to the vertex's normal,
			and stores the normalized reflected vector within the texture
			coordinates.

	Software Z buffer rendering.

	Separate projection, modelview, and texture matrices and matrix stacks.

	A linear texel filter.  This reduces aliasing seen within high-detailed textures
		and causes low-detailed textures to appear more smoothly.

	Repeatable and clamping border operations.

	Preventing rendering from occuring at the screen edges by ignoring edges
		that are fully outside the screen, and interpolating across edges
		that are partially inside the screen.  This way the pixel drawing
		code does not need to test for the pixel to be inside the screen
		before every pixel write.

	Support for multiple viewports per window and multiple views within a scene.
		Somewhat similar to the layout of a scene graph.

	Support for additive blending with the current color buffer pixel when rendering.

I have not yet done:

	I originally intended on implementing a color matrix.
		This matrix would transform all color tuples passed to vertices using
		osuColor3f() calls.  I have not yet implemented it, however doing so
		would only use the same functionality as the modelview, projection,
		and texture matrix stacks.

	Last I checked, for some odd reason 1D texture rendering wouldn't work.
		Luckily creating a 2D texture with a height of 1 texel is equivalent to
		creating a 1D texture.  I would have still preferred to implement 1D
		textures, for the sake of completeness.

	I originally intended to implemente multi-texturing (rendering multiple texture
		layers onto a single object) but did not have enough time.  Multi-texturing
		is useful for rendering lightmaps, bump-mapping, detail maps, and many other
		visual effects onto the surface of an object.

	Currently the only Z buffer operation is to throw out pixels which have
		smaller depth values than the current buffer value.  I wanted to give the
		user the option of selecting which comparison operation (less,
		less-or-equal-to, greater, greater-or-equal-to, equal, and not equal)
		to use for depth testing.

	I wanted to implement polygon view clipping.  The operation of perspective texture
		interpolation depends upon values within the depth buffer, and therefore
		rendering textures with depth values close to zero will produce artifacting.
		If a near clip plane was implemented then this artifacting should go away.
		Also, creating clip planes for the top, bottom, left, and right view edges
		could potentially increase the speed of the renderer.

	I originally planned to implement a Perlin-Noise 3D texture generation function,
		as was shown in my presentation on texture mapping.  This would have been
		an excellent demonstration for 3D texturing.

	I wanted to implement MIP-mapping, as well as automatic MIP-map generation.

	I added support additive blending when rendering, however OpenGL supports
		many more blending functions beyond additive blending.  I intended on
		implementing transparency blending.  Transparency blending involves
		interpolating in color space between the value currently in the color
		buffer and the value being written to the color buffer.  I also wanted
		to implement dot3 blending.  Dot3 blending involves taking the tuples
		of red, green, and blue for the current color within the color buffer and
		the new color to be written, re-mapping the range of their values from
		[0,255] to [-1,1], computing the dot product of these tuples, remapping the
		result product from [-1,1] to [0,255], and writing that remapped dot product
		value to all three color components within the color buffer.
		Dot-3 blending is another feature used for diffuse bump-map rendering.

	I haven't fully tested reflection texture coordinate generation.

	I haven't fully tested cube mapped textures.
		When combined with reflection texture coordinate generation, cube maps can
		potentially be used to render reflections of environments onto the surface
		of objects.
		Cube maps can also be used as an alternative method of quickly interpolating
		surface normal values across scanlines of a polygon.  Combining this
		technique with dot3 blending, cube maps can be used as a somewhat efficient
		method of computing phong shading.

Sources:

	OpenGL Manual Pages:
		http://www.opengl.org/documentation/specs/man_pages/hardcopy/GL/html/

	OpenGL Extension Registry:
		http://oss.sgi.com/projects/ogl-sample/registry/

	OpenGL Specifications:
		http://www.opengl.org/documentation/specs/version1.5/glspec15.pdf

	Hecker, Chris.  "Perspective Texture Mapping, Part 1: Foundations,"
		Game Developer, April 1995 pp. 16-25
		http://www.d6.com/users/checker/pdfs/gdmtex1.pdf

	Flavell, Andrew.  "Run-Time MIP-Map Filtering," Game Developer,
		November 1998
		http://www.gamasutra.com/features/19981211/flavell_01.htm

	Source code of the game, 'Quake,' released under the GPL.  Specifically, I used the
		OpenGL renderer of .MDL files.  No software rendering code from the
		Quake engine was examined.
		ftp://ftp.idsoftware.com/idstuff/source/q1source.zip

	Previously constructed OpenGL framework and .OBJ file loader from the CS 450 class.
