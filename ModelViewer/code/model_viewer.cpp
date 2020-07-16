#include <GL/glew.h>
#include "model_viewer_platform_common.h"
#include "model_viewer_math.h"
#include "model_viewer_shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <nfd.h>

#define INVALID_TEXTURE 0xFFFFFFFF
#define MAX_PATH 260

static GLuint
LoadTexture(const char* Path)
{
	GLuint TextureID;
	glGenTextures(1, &TextureID);

	int Width, Height, Channels;
	stbi_uc* Data = stbi_load(Path, &Width, &Height, &Channels, 0);
	if (Data)
	{
		GLenum Format;
		if (Channels == 1) Format = GL_RED;
		else if (Channels == 3) Format = GL_RGB;
		else if (Channels == 4) Format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, TextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, Format, Width, Height, 0, Format, GL_UNSIGNED_BYTE, Data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		TextureID = INVALID_TEXTURE;
		Assert(!"INVALID TEXTURE");
	}

	stbi_image_free(Data);

	return(TextureID);
}

struct mesh
{
	uint32_t BaseVertex;
	uint32_t BaseIndex;
	uint32_t IndexCount;
	uint32_t MaterialIndex;
};

enum vbo_type
{
	Pos_VBO,
	Normal_VBO,
	TexCoord_VBO,
	Index_VBO,

	Count_VBO
};
struct model
{
	GLuint VAO;
	GLuint VBOs[Count_VBO];

	std::vector<mesh> Meshes;
	std::vector<GLuint> Textures;
};

struct game_state
{
	bool IsInitialized;

	shader DefaultShader;

	model Model;
};

static void
Concatenate(char* Dest, char* SrcA, uint32_t SrcALength, char* SrcB, uint32_t SrcBLength)
{
	for (uint32_t I = 0; I < SrcALength; I++)
	{
		*Dest++ = *SrcA++;
	}

	for (uint32_t I = 0; I < SrcBLength; I++)
	{
		*Dest++ = *SrcB++;
	}

	*Dest = 0;
}

void
UpdateAndRender(game_memory* Memory, game_input* Input, uint32_t BufferWidth, uint32_t BufferHeight)
{
	Assert(sizeof(game_state) < Memory->PermanentStorageSize);
	game_state* GameState = (game_state*)Memory->PermanentStorage;
	if (!GameState->IsInitialized)
	{
		GameState->DefaultShader = shader("shaders\\DefaultVS.glsl", "shaders\\DefaultFS.glsl");

		model* Model = &GameState->Model;

		glGenVertexArrays(1, &Model->VAO);
		glGenBuffers(ArrayCount(Model->VBOs), Model->VBOs);
		glBindVertexArray(Model->VAO);

		char* ModelFilePath = 0;
		nfdresult_t result = NFD_OpenDialog(0, 0, &ModelFilePath);

		char* LastSlash = 0;
		for (char* C = ModelFilePath; *C != 0; C++)
		{
			if (*C == '\\')
			{
				LastSlash = C;
			}
		}
		uint32_t ModelDirLengthWithLastSlash = (uint32_t)(LastSlash - ModelFilePath) + 1;

		const aiScene* Scene = aiImportFile(ModelFilePath,
			aiProcess_Triangulate | aiProcess_GenSmoothNormals |
			aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
		if (Scene)
		{
			Model->Meshes.resize(Scene->mNumMeshes);
			Model->Textures.resize(Scene->mNumMaterials);

			uint32_t VertexCount = 0;
			uint32_t IndexCount = 0;
			for (uint32_t MeshIndex = 0;
				MeshIndex < Model->Meshes.size();
				MeshIndex++)
			{
				Model->Meshes[MeshIndex].BaseVertex = VertexCount;
				Model->Meshes[MeshIndex].BaseIndex = IndexCount;
				Model->Meshes[MeshIndex].IndexCount = 3 * Scene->mMeshes[MeshIndex]->mNumFaces;
				Model->Meshes[MeshIndex].MaterialIndex = Scene->mMeshes[MeshIndex]->mMaterialIndex;

				VertexCount += Scene->mMeshes[MeshIndex]->mNumVertices;
				IndexCount += Model->Meshes[MeshIndex].IndexCount;
			}

			std::vector<vec3> Positions, Normals;
			std::vector<vec2> TexCoords;
			std::vector<uint32_t> Indices;

			Positions.reserve(VertexCount);
			Normals.reserve(VertexCount);
			TexCoords.reserve(VertexCount);
			Indices.reserve(IndexCount);

			for (uint32_t MeshIndex = 0;
				MeshIndex < Model->Meshes.size();
				MeshIndex++)
			{
				const aiMesh* AssimpMesh = Scene->mMeshes[MeshIndex];

				for (uint32_t VertexIndex = 0;
					VertexIndex < AssimpMesh->mNumVertices;
					VertexIndex++)
				{
					const aiVector3D Pos = AssimpMesh->mVertices[VertexIndex];
					const aiVector3D Normal = AssimpMesh->HasNormals() ? AssimpMesh->mNormals[VertexIndex] : aiVector3D(0.0f);
					const aiVector3D TexCoord = AssimpMesh->HasTextureCoords(0) ? AssimpMesh->mTextureCoords[0][VertexIndex] : aiVector3D(0.0f);

					Positions.push_back(vec3(Pos.x, Pos.y, Pos.z));
					Normals.push_back(vec3(Normal.x, Normal.y, Normal.z));
					TexCoords.push_back(vec2(TexCoord.x, TexCoord.y));
				}

				for (uint32_t FaceIndex = 0;
					FaceIndex < AssimpMesh->mNumFaces;
					FaceIndex++)
				{
					const aiFace Face = AssimpMesh->mFaces[FaceIndex];
					Assert(Face.mNumIndices == 3);

					Indices.push_back(Face.mIndices[0]);
					Indices.push_back(Face.mIndices[1]);
					Indices.push_back(Face.mIndices[2]);
				}
			}

			for (uint32_t MaterialIndex = 0;
				MaterialIndex < Scene->mNumMaterials;
				MaterialIndex++)
			{
				const aiMaterial* AssimpMaterial = Scene->mMaterials[MaterialIndex];

				Model->Textures[MaterialIndex] = INVALID_TEXTURE;
				if (AssimpMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
				{
					aiString TexturePath;

					if (AssimpMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &TexturePath, 0, 0, 0, 0, 0) == AI_SUCCESS)
					{
						char* TextureName = TexturePath.data;
						for (char* C = TexturePath.data; *C != 0; C++)
						{
							if (*C == '\\')
							{
								TextureName = C + 1;
							}
						}
						uint32_t TextureNameLength = TexturePath.length - (uint32_t)(TextureName - TexturePath.data);

						char FullPath[MAX_PATH];
						Concatenate(FullPath, ModelFilePath, ModelDirLengthWithLastSlash, TextureName, TextureNameLength);
						Model->Textures[MaterialIndex] = LoadTexture(FullPath);
					}
				}
			}

			glBindBuffer(GL_ARRAY_BUFFER, Model->VBOs[Pos_VBO]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * Positions.size(), &Positions[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(Pos_VBO);
			glVertexAttribPointer(Pos_VBO, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glBindBuffer(GL_ARRAY_BUFFER, Model->VBOs[Normal_VBO]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(Normal_VBO);
			glVertexAttribPointer(Normal_VBO, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glBindBuffer(GL_ARRAY_BUFFER, Model->VBOs[TexCoord_VBO]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * TexCoords.size(), &TexCoords[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(TexCoord_VBO);
			glVertexAttribPointer(TexCoord_VBO, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Model->VBOs[Index_VBO]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * Indices.size(), &Indices[0], GL_STATIC_DRAW);
		}

		glBindVertexArray(0);
		GameState->IsInitialized = true;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 View = LookAt(vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 0.0f, 0.0f));
	mat4 PerspectiveProjection = Perspective(45.0f, (float)BufferWidth / (float)BufferHeight, 0.1f, 100.0f);
	mat4 Model = Translation(vec3(0.0f, -0.8f, 0.0f)) * Rotation(-90.0f, vec3(1.0f, 0.0f, 0.0f)) * Scaling(0.025f);
	GameState->DefaultShader.Use();
	GameState->DefaultShader.SetMat4("View", View);
	GameState->DefaultShader.SetMat4("Projection", PerspectiveProjection);
	GameState->DefaultShader.SetMat4("Model", Model);

	glBindVertexArray(GameState->Model.VAO);
	for (uint32_t MeshIndex = 0;
		MeshIndex < GameState->Model.Meshes.size();
		MeshIndex++)
	{
		mesh* Mesh = &GameState->Model.Meshes[MeshIndex];

		GLuint Texture = GameState->Model.Textures[Mesh->MaterialIndex];
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);

		glDrawElementsBaseVertex(GL_TRIANGLES, Mesh->IndexCount, GL_UNSIGNED_INT,
			(void*)(sizeof(uint32_t) * Mesh->BaseIndex),
			Mesh->BaseVertex);
	}
	glBindVertexArray(0);
}