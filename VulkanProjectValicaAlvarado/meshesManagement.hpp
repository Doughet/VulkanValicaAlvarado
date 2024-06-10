

/**************
 Creae the meshes, as described below
 
 WARNING!
 Since it is a C program, you can use for loops and functions if you think they can be helpful in your solution.
 However, please include all your code in this file, since it will be put in an automatic correction process
 for the final evaluation. Please also be cautious when using standard libraries and symbols, since they
 might not be available in all the development environments (especially, they might not be available
 in the final evaluation environment, preventing your code from compiling).
 This WARNING will be valid far ALL THE ASSIGNMENTs, but it will not be repeated in the following texts,
 so please remember these advices carefully!
 
***************/

void MakeSquare(float size, std::vector<std::array<float,6>> &vertices, std::vector<uint32_t> &indices) {
// Creates a square, on the xz-plane, aligned with the axis, and centered in the origin.
// The length of the four sides is in parameter >size<.
// The procedure should fill the array contained in the >vertices< parameter, with the positions of
// the vertices of the primitive, expressed with their local coordinates in a std::array<float,6> element.
// In particular, the first three elements (index 0,1,2) encode the position of the vertex (x in index 0,
// y in index 1, and z in index 2). The second set of three elements (index 3,4,5) encode the direction
// of the normal vector for the considerd vertex (N.x in index 3, N.y in index 4, and N.z in index 5).
// Indices should be returned in the >indices< array, starting from 0, and up to vertices.size()-1.
// The primitive is encoded as an indexed triangle list, so the size of the >indices< array, should
// be a multiple of 3: each group of three indices, defines a different triangle.
//
// HINT: this procedure has already been implemented. You can keep it as is
	vertices = {
				   {-size/2.0f,0.0f,-size/2.0f,0.0f,1.0f,0.0f},
				   {-size/2.0f,0.0f, size/2.0f,0.0f,1.0f,0.0f},
				   { size/2.0f,0.0f,-size/2.0f,0.0f,1.0f,0.0f},
				   { size/2.0f,0.0f, size/2.0f,0.0f,1.0f,0.0f}};
	indices = {0, 1, 2,    1, 3, 2};
}

void MakeCube(float size, std::vector<std::array<float,6>> &vertices, std::vector<uint32_t> &indices) {
// Creates a cube, with the faces perpendicular to the axis, and centered in the origin.
// The length of one edge of the cube is >size<.
// The procedure should fill the array contained in the >vertices< parameter, with the positions of
// the vertices of the primitive, expressed with their local coordinates in a std::array<float,6> element.
// In particular, the first three elements (index 0,1,2) encode the position of the vertex (x in index 0,
// y in index 1, and z in index 2). The second set of three elements (index 3,4,5) encode the direction
// of the normal vector for the considerd vertex (N.x in index 3, N.y in index 4, and N.z in index 5).
// Indices should be returned in the >indices< array, starting from 0, and up to vertices.size()-1.
// The primitive is encoded as an indexed triangle list, so the size of the >indices< array, should
// be a multiple of 3: each group of three indices, defines a different triangle.
//
// HINT: the procedure below creates a square. You can use it as a side of the cube (please remember
// to change the value of the y component, otherwise the result will be wrong
	/*vertices = {
				   {-size/2.0f,0.0f,-size/2.0f,0.0f,1.0f,0.0f},
				   {-size/2.0f,0.0f, size/2.0f,0.0f,1.0f,0.0f},
				   { size/2.0f,0.0f,-size/2.0f,0.0f,1.0f,0.0f},
				   { size/2.0f,0.0f, size/2.0f,0.0f,1.0f,0.0f}};
	indices = {0, 1, 2,    1, 3, 2};
    */
    float halfSize = size / 2.0f;

    // Relative positions of the vertices for each face of the cube
    std::vector<std::array<int, 3>> basePositions = {
            {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1}, // Front face
            {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}      // Back face
    };

    // Normals for each side
    std::vector<std::array<float, 3>> normals = {
            {0.0f, 0.0f, -1.0f}, // Front
            {0.0f, 0.0f, 1.0f},  // Back
            {0.0f, -1.0f, 0.0f}, // Down
            {0.0f, 1.0f, 0.0f},  // Up
            {-1.0f, 0.0f, 0.0f}, // Left side
            {1.0f, 0.0f, 0.0f}   // Right side
    };

    // Combinations of vertices for each face
    std::vector<std::array<int, 4>> faceVertices = {
            {0, 1, 2, 3}, // Front
            {5, 4, 7, 6}, // Back
            {4, 5, 1, 0}, // Down
            {3, 2, 6, 7}, // Up
            {4, 0, 3, 7}, // Left side
            {1, 5, 6, 2}  // Right side
    };

    // Create vertices with its positions and normals
    for (int i = 0; i < 6; ++i) {
        for (int j : faceVertices[i]) {
            std::array<float, 6> vertex = {
                    basePositions[j][0] * halfSize,
                    basePositions[j][1] * halfSize,
                    basePositions[j][2] * halfSize,
                    normals[i][0],
                    normals[i][1],
                    normals[i][2]
            };
            vertices.push_back(vertex);
        }
    }

    // Indixes for the cube faces
    for (int i = 0; i < 6; ++i) {
        int baseIndex = i * 4;
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 2);
    }
}

void MakeCylinder(float radius, float height, int slices, std::vector<std::array<float,6>> &vertices, std::vector<uint32_t> &indices) {
// Creates a cylinder, approximated by a prism with a base composed by a regular polygon with >slices< sides.
// The radius of the cylinder is >radius<, and it height is >height<. The cylinder has its centere
// in the origin, and spans half above and half below the plane that passes thorugh the origin.
// The top and bottom caps are are aligned with xz-plane and perpendicular to the y-axis.
// The procedure should fill the array contained in the >vertices< parameter, with the positions of
// the vertices of the primitive, expressed with their local coordinates in a std::array<float,6> element.
// In particular, the first three elements (index 0,1,2) encode the position of the vertex (x in index 0,
// y in index 1, and z in index 2). The second set of three elements (index 3,4,5) encode the direction
// of the normal vector for the considerd vertex (N.x in index 3, N.y in index 4, and N.z in index 5).
// Indices should be returned in the >indices< array, starting from 0, and up to vertices.size()-1.
// The primitive is encoded as an indexed triangle list, so the size of the >indices< array, should
// be a multiple of 3: each group of three indices, defines a different triangle.
//
// HINT: the procedure below creates a rectangle. You have to change it, or you will obtain a wrong result
// You should use a for loop, and you should start from the procedure to create a circle seen during the lesson
	/*vertices = {
				   {-radius,-height/2.0f,0.0f,0.0f,0.0f,1.0f},
				   {-radius, height/2.0f,0.0f,0.0f,0.0f,1.0f},
				   { radius,-height/2.0f,0.0f,0.0f,0.0f,1.0f},
				   { radius, height/2.0f,0.0f,0.0f,0.0f,1.0f}};
	indices = {0, 2, 1,    1, 2, 3};*/
    float halfHeight = height / 2.0f;
    float angleStep = 2 * M_PI / slices;

    //  Vertices and normals for the bottom and top cap
    for (int i = 0; i < slices; ++i) {
        float angle = i * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        // Vertices of the bottom cap
        vertices.push_back({x, -halfHeight, z, 0.0f, -1.0f, 0.0f});
        // Vertices of the top cap
        vertices.push_back({x, halfHeight, z, 0.0f, 1.0f, 0.0f});
    }

    // Cap centers
    vertices.push_back({0.0f, -halfHeight, 0.0f, 0.0f, -1.0f, 0.0f}); // Centro de la tapa inferior
    vertices.push_back({0.0f, halfHeight, 0.0f, 0.0f, 1.0f, 0.0f});  // Centro de la tapa superior

    // Indexes for the caps
    for (int i = 0; i < slices; ++i) {
        int next = (i + 1) % slices;

        // Bottom cap
        indices.push_back(i * 2);
        indices.push_back(next * 2);
        indices.push_back(vertices.size() - 2);

        // Top cap
        indices.push_back(i * 2 + 1);
        indices.push_back(vertices.size() - 1);
        indices.push_back(next * 2 + 1);
    }

    // Vertices and normals for the side surface
    int baseIndex = vertices.size();
    for (int i = 0; i < slices; ++i) {
        float angle = i * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        std::array<float, 3> normal = {cos(angle), 0.0f, sin(angle)};

        // Vertices of the side surface
        vertices.push_back({x, -halfHeight, z, normal[0], normal[1], normal[2]});
        vertices.push_back({x, halfHeight, z, normal[0], normal[1], normal[2]});
    }

    // Indexes of the side surface
    for (int i = 0; i < slices; ++i) {
        int next = (i + 1) % slices;

        // First triangle
        indices.push_back(baseIndex + i * 2);
        indices.push_back(baseIndex + i * 2 + 1);
        indices.push_back(baseIndex + next * 2);

        // Second triangle
        indices.push_back(baseIndex + i * 2 + 1);
        indices.push_back(baseIndex + next * 2 + 1);
        indices.push_back(baseIndex + next * 2);
    }
}

void MakeCone(float radius, float height, int slices, std::vector<std::array<float,6>> &vertices, std::vector<uint32_t> &indices) {
// Creates a cone, approximated by a pyramid with a base composed by a regular polygon with >slices< sides.
// The radius of the cone is >radius<, and it height is >height<. The cone has its centere
// in the origin, and spans half above and half below the plane that passes thorugh the origin.
// The bottom cap is aligned with xz-plane and perpendicular to the y-axis.
// The procedure should fill the array contained in the >vertices< parameter, with the positions of
// the vertices of the primitive, expressed with their local coordinates in a std::array<float,6> element.
// In particular, the first three elements (index 0,1,2) encode the position of the vertex (x in index 0,
// y in index 1, and z in index 2). The second set of three elements (index 3,4,5) encode the direction
// of the normal vector for the considerd vertex (N.x in index 3, N.y in index 4, and N.z in index 5).
// Indices should be returned in the >indices< array, starting from 0, and up to vertices.size()-1.
// The primitive is encoded as an indexed triangle list, so the size of the >indices< array, should
// be a multiple of 3: each group of three indices, defines a different triangle.
//
// HINT: the procedure below creates a triangle. You have to change it, or you will obtain a wrong result
// You should use a for loop, and you should start from the procedure to create a circle seen during the lesson
/*  vertices = {
				   {-radius,-height/2.0f,0.0f,0.0f,0.0f,1.0f},
				   { radius,-height/2.0f,0.0f,0.0f,0.0f,1.0f},
				   { 0.0f,   height/2.0f,0.0f,0.0f,0.0f,1.0f}};
	indices = {0, 1, 2};    */
    // Generate vertices for the bottom center of the cone
    std::array<float, 6> bottomCenter = {0.0f, -height / 2.0f, 0.0f, 0.0f, -1.0f, 0.0f};
    vertices.push_back(bottomCenter);

    // Generate vertices for the base of the cone
    float angleStep = 2.0f * M_PI / slices;
    for (int i = 0; i < slices; ++i) {
        float angle = angleStep * i;
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);
        float normalX = std::cos(angle);
        float normalZ = std::sin(angle);
        std::array<float, 6> vertex = {x, -height / 2.0f, z, normalX, 0.0f, normalZ};
        vertices.push_back(vertex);
    }

    // Generate vertices for the top of the cone
    std::array<float, 6> top = {0.0f, height / 2.0f, 0.0f, 0.0f, 1.0f, 0.0f};
    vertices.push_back(top);

    // Generate indices for the base of the cone
    for (int i = 1; i <= slices; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back((i % slices) + 1);
    }

    // Generate indices for the side faces of the cone
    for (int i = 1; i <= slices; ++i) {
        indices.push_back(i);
        indices.push_back(slices + 1);
        indices.push_back((i % slices) + 1);
    }
}

void MakeSphere(float radius, int rings, int slices, std::vector<std::array<float,6>> &vertices, std::vector<uint32_t> &indices)
{
// Creates a sphere, approximated by a poliedron composed by >slices<, and >rings< rings.
// The radius of the sphere is >radius<, and it is centered in the origin.
// The procedure should fill the array contained in the >vertices< parameter, with the positions of
// the vertices of the primitive, expressed with their local coordinates in a std::array<float,6> element.
// In particular, the first three elements (index 0,1,2) encode the position of the vertex (x in index 0,
// y in index 1, and z in index 2). The second set of three elements (index 3,4,5) encode the direction
// of the normal vector for the considerd vertex (N.x in index 3, N.y in index 4, and N.z in index 5).
// Indices should be returned in the >indices< array, starting from 0, and up to vertices.size()-1.
// The primitive is encoded as an indexed triangle list, so the size of the >indices< array, should
// be a multiple of 3: each group of three indices, defines a different triangle.
//
// HINT: the procedure below creates a circle. You have to change it, or you will obtain a wrong result
// You should use two nested for loops, one used to span across the rings, and the other that spans along
// the rings.
/*	vertices.resize(slices+1);
	indices.resize(3*slices);
	vertices[slices] = {0.0f,0.0f,0.0f,0.0f,0.0f,1.0f};
	for(int i = 0; i < slices; i++) {
		float ang = 2*M_PI * (float)i / (float)slices;
		vertices[i] = {radius * cos(ang), radius * sin(ang), 0.0f,0.0f,0.0f,1.0f};
		indices[3*i  ] = slices;
		indices[3*i+1] = i;
		indices[3*i+2] = (i+1) % slices;
	}*/
    vertices.clear();
    indices.clear();

    // Generate vertices
    for (int i = 0; i <= rings; ++i) {
        float theta = i * M_PI / rings;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int j = 0; j < slices; ++j) {
            float phi = j * 2.0f * M_PI / slices;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float x = radius * cosPhi * sinTheta;
            float y = radius * cosTheta;
            float z = radius * sinPhi * sinTheta;

            std::array<float, 6> vertex = {x, y, z, x / radius, y / radius, z / radius};
            vertices.push_back(vertex);
        }
    }

    // Generate indices
    for (int i = 0; i < rings; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = (i * slices) + j;
            int second = first + slices;

            indices.push_back(first);
            indices.push_back((second + 1) % (rings * slices));
            indices.push_back(second);

            indices.push_back(first);
            indices.push_back((first + 1) % (rings * slices));
            indices.push_back((second + 1) % (rings * slices));
        }
    }
}
