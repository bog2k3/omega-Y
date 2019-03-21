#include "Terrain.h"

#include "triangulation.h"
#include "HeightMap.h"
#include "PerlinNoise.h"
#include "Water.h"
#include "../CustomRenderContext.h"

#include "../BSP/BSPDebugDraw.h"

#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/renderOpenGL/shader.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/renderOpenGL/TextureLoader.h>
#include <boglfw/math/math3D.h>
#include <boglfw/World.h>
#include <boglfw/utils/rand.h>
#include <boglfw/utils/log.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <bullet3/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <bullet3/BulletDynamics/Dynamics/btRigidBody.h>
#include <bullet3/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

#include <GL/glew.h>

#include <new>
#include <algorithm>

// TODO: optimize texture mapping with atlas and texture array: https://www.khronos.org/opengl/wiki/Array_Texture

struct Terrain::TerrainVertex {
	static const unsigned nTextures = 5;

	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 uv[nTextures];	// uvs for each texture layer
	glm::vec4 texBlendFactor;	// 4 texture blend factors: x is between grass1 & grass2,
								// 							y between rock1 & rock2
								//							z between grass/sand and rock (highest priority)
								//							w between grass and sand

	TerrainVertex() = default;
};

struct TextureInfo {
	unsigned texID = 0;		// GL texture ID
	float wWidth = 1.f;		// width of texture in world units (meters)
	float wHeight = 1.f;	// height/length of texture in world units (meters)

	~TextureInfo() {
		if (texID)
			glDeleteTextures(1, &texID);
	}
};

struct Terrain::RenderData {
	unsigned VAO_;
	unsigned VBO_;
	unsigned IBO_;
	unsigned trisBelowWater_;
	unsigned trisAboveWater_;
	int shaderProgram_;
	int iPos_;
	int iNormal_;
	int iColor_;
	int iUV_;
	int iTexBlendF_;
	int imPV_;
	int iSampler_;
	int iEyePos_;
	int iSubspace_;
	int ibRefraction_;
	TextureInfo textures_[TerrainVertex::nTextures];
};

template<>
float nth_elem(Terrain::TerrainVertex const& v, unsigned n) {
	return	n==0 ? v.pos.x :
			n==1 ? v.pos.z :
			0.f;
}

class TriangleAABBGenerator : public AABBGeneratorInterface<unsigned> {
public:
	TriangleAABBGenerator(Terrain* terrain) : pTerrain_(terrain) {}
	virtual AABB getAABB(unsigned const& i) override {
		// compute the AABB for terrain triangle at index i;
		// TODO: speed up by caching AABBs for triangles
		glm::vec3* p1 = &pTerrain_->pVertices_[pTerrain_->triangles_[i].iV1].pos;
		glm::vec3* p2 = &pTerrain_->pVertices_[pTerrain_->triangles_[i].iV2].pos;
		glm::vec3* p3 = &pTerrain_->pVertices_[pTerrain_->triangles_[i].iV3].pos;
		AABB ret(*p1, *p1);
		ret.expand(*p2);
		ret.expand(*p3);
		return ret;
	}
private:
	Terrain* pTerrain_;
};

Terrain::Terrain()
	: physicsBodyMeta_(this)
{
	LOGPREFIX("Terrain");

	renderData_ = new RenderData();
	glGenVertexArrays(1, &renderData_->VAO_);
	glGenBuffers(1, &renderData_->VBO_);
	glGenBuffers(1, &renderData_->IBO_);

	Shaders::createProgram("data/shaders/terrain.vert", "data/shaders/terrain.frag", [this](unsigned id) {
		renderData_->shaderProgram_ = id;
		if (!renderData_->shaderProgram_) {
			ERROR("Failed to load terrain shaders!");
			return;
		}
		renderData_->iPos_ = glGetAttribLocation(renderData_->shaderProgram_, "pos");
		renderData_->iNormal_ = glGetAttribLocation(renderData_->shaderProgram_, "normal");
		renderData_->iColor_ = glGetAttribLocation(renderData_->shaderProgram_, "color");
		renderData_->iUV_ = glGetAttribLocation(renderData_->shaderProgram_, "uv");
		renderData_->iTexBlendF_ = glGetAttribLocation(renderData_->shaderProgram_, "texBlendFactor");
		renderData_->imPV_ = glGetUniformLocation(renderData_->shaderProgram_, "mPV");
		renderData_->iEyePos_ = glGetUniformLocation(renderData_->shaderProgram_, "eyePos");
		renderData_->iSampler_ = glGetUniformLocation(renderData_->shaderProgram_, "tex");
		renderData_->iSubspace_ = glGetUniformLocation(renderData_->shaderProgram_, "subspace");
		renderData_->ibRefraction_ = glGetUniformLocation(renderData_->shaderProgram_, "bRefraction");

		glBindVertexArray(renderData_->VAO_);
		glBindBuffer(GL_ARRAY_BUFFER, renderData_->VBO_);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderData_->IBO_);
		glEnableVertexAttribArray(renderData_->iPos_);
		glVertexAttribPointer(renderData_->iPos_, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex),
			(void*)offsetof(TerrainVertex, pos));
		if (renderData_->iNormal_ > 0) {
			glEnableVertexAttribArray(renderData_->iNormal_);
			glVertexAttribPointer(renderData_->iNormal_, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex),
				(void*)offsetof(TerrainVertex, normal));
		}
		if (renderData_->iColor_ > 0) {
			glEnableVertexAttribArray(renderData_->iColor_);
			glVertexAttribPointer(renderData_->iColor_, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex),
				(void*)offsetof(TerrainVertex, color));
		}
		if (renderData_->iUV_ > 0) {
			for (unsigned i=0; i<TerrainVertex::nTextures; i++) {
				glEnableVertexAttribArray(renderData_->iUV_ + i);
				glVertexAttribPointer(renderData_->iUV_ + i, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex),
					(void*)(offsetof(TerrainVertex, uv) + i*sizeof(TerrainVertex::uv[0])));
			}
		}
		if (renderData_->iTexBlendF_ > 0) {
			glEnableVertexAttribArray(renderData_->iTexBlendF_);
			glVertexAttribPointer(renderData_->iTexBlendF_, 4, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex),
				(void*)offsetof(TerrainVertex, texBlendFactor));
		}
		glBindVertexArray(0);
	});

	loadTextures();

	pWater_ = new Water();

	triangleAABBGenerator_ = new TriangleAABBGenerator(this);
}

Terrain::~Terrain()
{
	clear();
	delete renderData_, renderData_ = nullptr;
	delete pWater_, pWater_ = nullptr;
	delete triangleAABBGenerator_, triangleAABBGenerator_ = nullptr;
}

void Terrain::clear() {
	if (pVertices_)
		free(pVertices_), pVertices_ = nullptr, nVertices_ = 0;
	triangles_.clear();
	physicsBodyMeta_.reset();
	if (heightFieldValues_)
		free(heightFieldValues_), heightFieldValues_ = nullptr;
	if (pBSP_)
		delete pBSP_, pBSP_ = nullptr;
}

void Terrain::loadTextures() {
	LOGLN("Loading textures . . .");
	renderData_->textures_[0].texID = TextureLoader::loadFromPNG("data/textures/terrain/dirt3.png", true);
	glBindTexture(GL_TEXTURE_2D, renderData_->textures_[0].texID);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	renderData_->textures_[0].wWidth = 2.f;
	renderData_->textures_[0].wHeight = 2.f;

	renderData_->textures_[1].texID = TextureLoader::loadFromPNG("data/textures/terrain/grass1.png", true);
	glBindTexture(GL_TEXTURE_2D, renderData_->textures_[1].texID);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	renderData_->textures_[1].wWidth = 3.f;
	renderData_->textures_[1].wHeight = 3.f;

	renderData_->textures_[2].texID = TextureLoader::loadFromPNG("data/textures/terrain/rock1.png", true);
	glBindTexture(GL_TEXTURE_2D, renderData_->textures_[2].texID);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	renderData_->textures_[2].wWidth = 3.f;
	renderData_->textures_[2].wHeight = 3.f;

	renderData_->textures_[3].texID = TextureLoader::loadFromPNG("data/textures/terrain/rock3.png", true);
	glBindTexture(GL_TEXTURE_2D, renderData_->textures_[3].texID);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	renderData_->textures_[3].wWidth = 4.f;
	renderData_->textures_[3].wHeight = 4.f;

	renderData_->textures_[4].texID = TextureLoader::loadFromPNG("data/textures/terrain/sand1.png", true);
	glBindTexture(GL_TEXTURE_2D, renderData_->textures_[4].texID);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	renderData_->textures_[4].wWidth = 4.f;
	renderData_->textures_[4].wHeight = 4.f;

	glBindTexture(GL_TEXTURE_2D, 0);

	LOGLN("Textures loaded.");
}

void validateSettings(TerrainConfig const& s) {
	assert(s.width > 0);
	assert(s.length > 0);
	assert(s.maxElevation > s.minElevation);
	assert(s.vertexDensity > 0);
	assert(s.width >= 1.f / s.vertexDensity);
	assert(s.length >= 1.f / s.vertexDensity);
}

void Terrain::generate(TerrainConfig const& settings) {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	LOGLN("Generating terrain . . .");
	validateSettings(settings);
	clear();
	config_ = settings;

	rows_ = (unsigned)ceil(config_.length * config_.vertexDensity) + 1;
	cols_ = (unsigned)ceil(config_.width * config_.vertexDensity) + 1;

	// we need to generate some 'skirt' vertices that will encompass the entire terrain in a circle,
	// in order to extend the sea-bed away from the main terrain
	float terrainRadius = sqrtf(config_.width * config_.width + config_.length * config_.length) * 0.5f;
	float seaBedRadius = terrainRadius * 2.5f;
	float skirtVertSpacing = 30.f; // meters
	unsigned nSkirtVerts = (2 * PI * seaBedRadius) / skirtVertSpacing;
	float skirtVertSector = 2 * PI / nSkirtVerts; // sector size between two skirt vertices
	nVertices_ = rows_ * cols_ + nSkirtVerts;
	pVertices_ = (TerrainVertex*)malloc(sizeof(TerrainVertex) * nVertices_);

	glm::vec3 bottomLeft {-config_.width * 0.5f, 0.f, -settings.length * 0.5f};
	float dx = config_.width / (cols_ - 1);
	float dz = config_.length / (rows_ - 1);
	gridSpacing_ = {dx, dz};
	// compute terrain vertices
	for (unsigned i=0; i<rows_; i++)
		for (unsigned j=0; j<cols_; j++) {
			glm::vec2 jitter { randf() * config_.relativeRandomJitter * dx, randf() * config_.relativeRandomJitter * dz };
			new(&pVertices_[i*cols_ + j]) TerrainVertex {
				bottomLeft + glm::vec3(dx * j + jitter.x, config_.minElevation, dz * i + jitter.y),	// position
				{0.f, 1.f, 0.f},																	// normal
				{1.f, 1.f, 1.f},																	// color
				{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},						// uvs
				{0.f, 0.f, 0.f, 0.f}																// tex blend factor
			};
			// compute UVs
			for (unsigned t=0; t<TerrainVertex::nTextures; t++) {
				pVertices_[i*cols_ + j].uv[t].x = (pVertices_[i*cols_ + j].pos.x - bottomLeft.x) / renderData_->textures_[t].wWidth;
				pVertices_[i*cols_ + j].uv[t].y = (pVertices_[i*cols_ + j].pos.z - bottomLeft.z) / renderData_->textures_[t].wHeight;
			}
		}
	// compute skirt vertices
	for (unsigned i=0; i<nSkirtVerts; i++) {
		float x = seaBedRadius * cosf(i*skirtVertSector);
		float z = seaBedRadius * sinf(i*skirtVertSector);
		new(&pVertices_[rows_*cols_+i]) TerrainVertex {
			{ x, config_.minElevation, z },								// position
			{ 0.f, 1.f, 0.f },												// normal
			{ 1.f, 1.f, 1.f },												// color
			{ {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f} },	// uvs
			{ 0.f, 0.f, 0.f, 1.f }											// tex blend factor
		};
		// compute UVs
		for (unsigned t=0; t<TerrainVertex::nTextures; t++) {
			pVertices_[rows_*cols_ + i].uv[t].x = (x - bottomLeft.x) / renderData_->textures_[t].wWidth;
			pVertices_[rows_*cols_ + i].uv[t].y = (z - bottomLeft.z) / renderData_->textures_[t].wHeight;
		}
	}

	LOGLN("Triangulating . . .");
	int trRes = triangulate(pVertices_, nVertices_, triangles_);
	if (trRes < 0) {
		ERROR("Failed to triangulate terrain mesh!");
		return;
	}
	fixTriangleWinding();	// after triangulation some triangles are ccw, we need to fix them

	LOGLN("Computing displacements . . .");
	computeDisplacements();
	LOGLN("Computing normals . . .");
	computeNormals();
	LOGLN("Computing texture weights . . .");
	computeTextureWeights();

	LOGLN("Creating Binary Space Partitioning tree . . .")
	std::vector<unsigned> triIndices;
	triIndices.reserve(triangles_.size());
	for (unsigned i=0; i<triangles_.size(); i++)
		triIndices.push_back(i);
	BSPConfig bspConfig;
	bspConfig.maxDepth = glm::ivec3{ 100, 1, 100 };
	bspConfig.minCellSize = glm::vec3{ 2.f, 1000.f, 2.f };
	bspConfig.minObjects = 5;
	bspConfig.targetVolume = AABB({-config_.width*0.5f, config_.minElevation - 10.f, -config_.length * 0.5f},
								{+config_.width*0.5f, config_.maxElevation + 10.f, +config_.length * 0.5f});
	bspConfig.dynamic = false;
	pBSP_ = new BSPTree<unsigned>(bspConfig, triangleAABBGenerator_, std::move(triIndices));

	LOGLN("Generating water . . .");
	pWater_->generate(WaterParams {
		terrainRadius,					// inner radius
		seaBedRadius - terrainRadius + 200,// outer extent
		max(0.05f, 2.f / terrainRadius),// vertex density
		false							// constrain to circle
	});
	LOGLN("Done generating.");
}

void Terrain::finishGenerate() {
	LOGPREFIX("Terrain");
	LOGLN("Updating render and physics objects . . .");
	updateRenderBuffers();
	updatePhysics();
}

void Terrain::fixTriangleWinding() {
	// all triangles must be CW as seen from above
	for (auto &t : triangles_) {
		glm::vec3 n = glm::cross(pVertices_[t.iV2].pos - pVertices_[t.iV1].pos, pVertices_[t.iV3].pos - pVertices_[t.iV1].pos);
		if (n.y < 0) {
			// triangle is CCW, we need to reverse it
			xchg(t.iV1, t.iV3);	// exchange vertices 1 and 3
			xchg(t.iN12, t.iN23); // exchange edges 1-2 and 2-3
		}
	}
}

void Terrain::computeDisplacements() {
	HeightmapParams hparam;
	hparam.width = max(4.f, config_.width / 8);
	hparam.length = max(4.f, config_.length / 8);
	hparam.minHeight = config_.minElevation;
	hparam.maxHeight = config_.maxElevation;
	HeightMap height(hparam);
	PerlinNoise smallNoise(config_.width, config_.length);
	PerlinNoise bigNoise(max(4.f, config_.width / 50), max(4.f, config_.length / 50));

	glm::vec3 bottomLeft {-config_.width * 0.5f, 0.f, -config_.length * 0.5f};
	for (unsigned i=1; i<rows_-1; i++) // we leave the edge vertices at zero to avoid artifacts with the skirt
		for (unsigned j=1; j<cols_-1; j++) {
			unsigned k = i*cols_ + j;
			float u = (pVertices_[k].pos.x - bottomLeft.x) / config_.width;
			float v = (pVertices_[k].pos.z - bottomLeft.z) / config_.length;

			float perlinAmp = (config_.maxElevation - config_.minElevation) * 0.1f;
			float hiFreq = //smallNoise.get(u/8, v/8, 1.f) * perlinAmp * 0.5
							+ smallNoise.get(u/4, v/4, 1.f) * perlinAmp * 0.3
							+ smallNoise.get(u/2, v/2, 1.f) * perlinAmp * 0.15
							+ smallNoise.get(u/1, v/1, 1.f) * perlinAmp * 0.05;
			float lowFreq = height.value(u, v);
			float lowFreqSmooth = bigNoise.getNorm(u, v, 1.f) * (config_.maxElevation - config_.minElevation) + config_.minElevation;
			lowFreq = lerp(lowFreqSmooth, lowFreq, config_.bigRoughness);

			pVertices_[k].pos.y = lowFreq + hiFreq * config_.smallRoughness;

			// TODO : use vertex colors with perlin noise for more variety

			// debug
			//float hr = (pVertices_[k].pos.y - config_.minElevation) / (config_.maxElevation - config_.minElevation);
			//pVertices_[k].color = {1.f - hr, hr, 0};
		}
	meltEdges(cols_ * 0.1, rows_ * 0.1);
}

void Terrain::meltEdges(unsigned xRadius, unsigned zRadius) {
	if (xRadius > cols_/2 || zRadius > rows_/2) {
		ERROR("Terrain::meltEdges() received invalid parameter");
		return;
	}
	auto slopeFn = [](float x) {
		return 0.5f + 0.5f * sinf(x * PI - PI/2);
	};
	// top edge:
	for (unsigned i=0; i<zRadius; i++) {
		float f = (float)i / zRadius;
		f = slopeFn(f);
		for (unsigned j=0; j<cols_; j++)
			pVertices_[i*cols_+j].pos.y = lerp(config_.minElevation, pVertices_[i*cols_+j].pos.y, f);
	}
	// bottom edge:
	for (unsigned i=rows_-zRadius; i<rows_; i++) {
		float f = (float)(rows_-i) / zRadius;
		f = slopeFn(f);
		for (unsigned j=0; j<cols_; j++)
			pVertices_[i*cols_+j].pos.y = lerp(config_.minElevation, pVertices_[i*cols_+j].pos.y, f);
	}
	// left edge:
	for (unsigned j=0; j<xRadius; j++) {
		float f = (float)j / xRadius;
		f = slopeFn(f);
		for (unsigned i=0; i<rows_; i++)
			pVertices_[i*cols_+j].pos.y = lerp(config_.minElevation, pVertices_[i*cols_+j].pos.y, f);
	}
	// right edge:
	for (unsigned j=cols_-xRadius; j<cols_; j++) {
		float f = (float)(cols_-j) / xRadius;
		f = slopeFn(f);
		for (unsigned i=0; i<rows_; i++)
			pVertices_[i*cols_+j].pos.y = lerp(config_.minElevation, pVertices_[i*cols_+j].pos.y, f);
	}
}

void Terrain::computeNormals() {
	for (auto &t : triangles_) {
		glm::vec3 n = glm::cross(pVertices_[t.iV2].pos - pVertices_[t.iV1].pos, pVertices_[t.iV3].pos - pVertices_[t.iV1].pos);
		n = glm::normalize(n);
		pVertices_[t.iV1].normal += n;
		pVertices_[t.iV2].normal += n;
		pVertices_[t.iV3].normal += n;
	}
	for (unsigned i=0; i<nVertices_; i++) {
		if ((i/cols_) == 0 || (i/cols_) == rows_-1 || (i%cols_) == 0 || (i%cols_) == cols_-1)
			pVertices_[i].normal = glm::vec3{0.f, 1.f, 0.f};  // we leave the edge vertices at zero to avoid artifacts with the skirt
		else
			pVertices_[i].normal = glm::normalize(pVertices_[i].normal);
	}
}

void Terrain::computeTextureWeights() {
	PerlinNoise pnoise(config_.width/2, config_.length/2);
	glm::vec3 bottomLeft {-config_.width * 0.5f, 0.f, -config_.length * 0.5f};
	const float grassBias = 0.2f; // bias to more grass over dirt
	for (unsigned i=0; i<rows_*cols_; i++) {
		// grass/rock factor is determined by slope
		// each one of grass and rock have two components blended together by a perlin factor for low-freq variance
		float u = (pVertices_[i].pos.x - bottomLeft.x) / config_.width;
		float v = (pVertices_[i].pos.z - bottomLeft.z) / config_.length;
		// #1 Grass vs Dirt factor
		pVertices_[i].texBlendFactor.x = grassBias
											+ pnoise.getNorm(u*0.15, v*0.15, 7.f)
											+ 0.3f * pnoise.get(u*0.3, v*0.3, 7.f)
											+ 0.1 * pnoise.get(u*0.6, v*0.6, 2.f);	// dirt / grass
		// #2 Rock1 vs Rock2 factor
		pVertices_[i].texBlendFactor.y = pnoise.getNorm(v*0.15, u*0.15, 7.f) + 0.5 * pnoise.get(v*0.6, u*0.6, 2.f);	// rock1 / rock2

		// #3 Rock vs Grass/Sand factor (highest priority)
		float cutoffY = 0.80f;	// y-component of normal above which grass is used instead of rock
		if (pVertices_[i].pos.y > 0) {
			// above water grass-rock coefficient
			// height factor for grass vs rock: the higher the vertex, the more likely it is to be rock
			float hFactor = clamp(pVertices_[i].pos.y / config_.maxElevation, 0.f, 1.f); // hFactor is 1.0 at the highest elevation, 0.0 at sea level.
			hFactor = pow(hFactor, 1.5f);
			cutoffY += (1.0 - cutoffY) * hFactor;
			pVertices_[i].texBlendFactor.z = pVertices_[i].normal.y > cutoffY ? 1.f : 0.f; // grass vs rock
		} else {
			// this is below water
			if (u <= 0.01 || v <= 0.01 || u >= 0.99 || v >= 0.99) {
				// edges are always sand
				pVertices_[i].texBlendFactor.z = 1.f;
			} else {
				if (pVertices_[i].normal.y > cutoffY + 0.1f) {
					// below water rock coefficient based on perlin noise
					float noise = pnoise.get(u*0.15, v*0.15, 7.f)
								+ 0.3 * pnoise.get(u*0.6, v*0.6, 7.f)
								+ 0.1 * pnoise.get(u*1.0, v*1.0, 7.f);
					float sandBias = 0.4f * pVertices_[i].normal.y; // flat areas are more likely to be sand rather than rock
					pVertices_[i].texBlendFactor.z = noise + sandBias > 0 ? 1.f : 0.25f;
				} else
					pVertices_[i].texBlendFactor.z = 0.2f; // steep underwater areas are still rock
			}
		}

		// #4 Grass vs Sand factor -> some distance above water level and everything below is sand or rock
		float beachHeight = 1.f + 1.5f * pnoise.getNorm(u*1.5, v*1.5, 1.f); // meters
		if (pVertices_[i].pos.y < beachHeight) {
			float sandFactor = min(1.f, beachHeight - pVertices_[i].pos.y);
			pVertices_[i].texBlendFactor.w = pow(sandFactor, 1.5f);
		} else
			pVertices_[i].texBlendFactor.w = 0;
	}
}

void Terrain::updateRenderBuffers() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	glBindBuffer(GL_ARRAY_BUFFER, renderData_->VBO_);
	glBufferData(GL_ARRAY_BUFFER, nVertices_ * sizeof(TerrainVertex), pVertices_, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	uint32_t *indices = (uint32_t*)malloc(6 * triangles_.size() * sizeof(uint32_t));	// allocate twice as much space to make sure we don't overrun
																						// the buffer when computing above/below water triangles
	renderData_->trisBelowWater_ = 0;
	float waterLevelTolerance = 1.f;
	// first loop: indices for tris below water
	for (unsigned i=0; i<triangles_.size(); i++) {
		// decide if triangle is at least partially submerged:
		if (pVertices_[triangles_[i].iV1].pos.y >= waterLevelTolerance
			&& pVertices_[triangles_[i].iV2].pos.y >= waterLevelTolerance
			&& pVertices_[triangles_[i].iV3].pos.y >= waterLevelTolerance)
			continue;
		indices[renderData_->trisBelowWater_*3 + 0] = triangles_[i].iV1;
		indices[renderData_->trisBelowWater_*3 + 1] = triangles_[i].iV2;
		indices[renderData_->trisBelowWater_*3 + 2] = triangles_[i].iV3;
		renderData_->trisBelowWater_++;
	}
	// second loop: indices for tris above water
	renderData_->trisAboveWater_ = 0;
	for (unsigned i=0; i<triangles_.size(); i++) {
		// check if the triangle is at least partially above water
		if (pVertices_[triangles_[i].iV1].pos.y <= -waterLevelTolerance
			&& pVertices_[triangles_[i].iV2].pos.y <= -waterLevelTolerance
			&& pVertices_[triangles_[i].iV3].pos.y <= -waterLevelTolerance)
			continue;
		indices[renderData_->trisBelowWater_*3 + renderData_->trisAboveWater_*3 + 0] = triangles_[i].iV1;
		indices[renderData_->trisBelowWater_*3 + renderData_->trisAboveWater_*3 + 1] = triangles_[i].iV2;
		indices[renderData_->trisBelowWater_*3 + renderData_->trisAboveWater_*3 + 2] = triangles_[i].iV3;
		renderData_->trisAboveWater_++;
	}
	assertDbg(renderData_->trisBelowWater_ + renderData_->trisAboveWater_ >= triangles_.size());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderData_->IBO_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * (renderData_->trisBelowWater_ + renderData_->trisAboveWater_) * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	free(indices);
}

void Terrain::updatePhysics() {
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	// create array of height values:
	if (heightFieldValues_)
		free(heightFieldValues_), heightFieldValues_ = nullptr;
	heightFieldValues_ = (float*)malloc(sizeof(float) * rows_ * cols_);
	glm::vec3 bottomLeft {-config_.width * 0.5f, 0.f, -config_.length * 0.5f};
	float dx = config_.width / (cols_ - 1);
	float dz = config_.length / (rows_ - 1);
	const float heightOffset = 0.1f;	// offset physics geometry slightly higher
	for (unsigned i=0; i<rows_; i++)
		for (unsigned j=0; j<cols_; j++)
			heightFieldValues_[i*cols_+j] = getHeightValue(bottomLeft + glm::vec3{j*dx, 0, i*dz}) + heightOffset;

	// create ground body
	physicsBodyMeta_.reset();
	PhysBodyConfig bodyCfg;
	bodyCfg.position = glm::vec3{0.f, (config_.maxElevation + config_.minElevation)*0.5f, 0.f};
	bodyCfg.mass = 0.f;
	bodyCfg.friction = 0.5f;
	bodyCfg.shape = std::make_shared<btHeightfieldTerrainShape>(cols_, rows_, heightFieldValues_, 1.f,
							config_.minElevation, config_.maxElevation, 1, PHY_FLOAT, false);

	physicsBodyMeta_.createBody(bodyCfg);
}

void Terrain::draw(RenderContext const& ctx) {
	if (!renderData_->shaderProgram_) {
		return;
	}

	auto const& rctx = CustomRenderContext::fromCtx(ctx);

	if (rctx.renderPass == RenderPass::Standard || rctx.renderPass == RenderPass::WaterReflection || rctx.renderPass == RenderPass::WaterRefraction) {
		if (renderWireframe_) {
			glLineWidth(2.f);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		// set-up textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderData_->textures_[0].texID);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, renderData_->textures_[1].texID);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, renderData_->textures_[2].texID);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, renderData_->textures_[3].texID);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, renderData_->textures_[4].texID);
		// configure backface culling
		glFrontFace(GL_CW);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);
		glEnable(GL_CLIP_DISTANCE0);
		// set-up shader, vertex buffer and uniforms
		glUseProgram(renderData_->shaderProgram_);
		glUniformMatrix4fv(renderData_->imPV_, 1, GL_FALSE, glm::value_ptr(ctx.viewport.camera().matProjView()));
		glUniform3fv(renderData_->iEyePos_, 1, &ctx.viewport.camera().position().x);
		for (unsigned i=0; i<TerrainVertex::nTextures; i++)
			glUniform1i(renderData_->iSampler_ + i, i);
		glBindVertexArray(renderData_->VAO_);
		glUniform1f(renderData_->iSubspace_, rctx.clipPlane.y);
		glUniform1i(renderData_->ibRefraction_, rctx.renderPass == RenderPass::WaterRefraction ? 1 : 0);
		if (rctx.clipPlane.y < 0) {
			// draw below-water subspace:
			glDrawElements(GL_TRIANGLES, renderData_->trisBelowWater_ * 3, GL_UNSIGNED_INT, nullptr);
		} else {
			// draw above-water subspace:
			glDrawElements(GL_TRIANGLES, renderData_->trisAboveWater_ * 3, GL_UNSIGNED_INT, (void*)(renderData_->trisBelowWater_*3*4));
		}
		// unbind stuff}
		glBindVertexArray(0);
		glUseProgram(0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_CULL_FACE);
		glDisable(GL_CLIP_DISTANCE0);

		if (renderWireframe_) {	// reset state
			glLineWidth(1.f);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		// draw vertex normals
		/*for (unsigned i=0; i<nVertices_; i++) {
			Shape3D::get()->drawLine(pVertices_[i].pos, pVertices_[i].pos+pVertices_[i].normal, {1.f, 0, 1.f});
		}*/
		//BSPDebugDraw::draw(*pBSP_);
		//for (unsigned i=0; i<triangles_.size() / 10; i++)
		//	Shape3D::get()->drawAABB(triangleAABBGenerator_->getAABB(i), glm::vec3{0.f, 1.f, 0.f});
	} else if (!renderWireframe_ && rctx.renderPass == RenderPass::WaterSurface) {
		pWater_->draw(ctx);
	}
}

float Terrain::getHeightValue(glm::vec3 const& where) const {
	auto *node = pBSP_->getNodeAtPoint(where);
	glm::vec3 intersectionPoint;
	glm::vec3 rayStart{where.x, config_.maxElevation + 100, where.z};
	glm::vec3 rayDir{0.f, -1.f, 0.f};
	for (unsigned tIndex : node->objects()) {
		glm::vec3 &p1 = pVertices_[triangles_[tIndex].iV1].pos;
		glm::vec3 &p2 = pVertices_[triangles_[tIndex].iV2].pos;
		glm::vec3 &p3 = pVertices_[triangles_[tIndex].iV3].pos;
		if (rayIntersectTri(rayStart, rayDir, p1, p2, p3, intersectionPoint))
			return intersectionPoint.y;
	}
	return config_.minElevation;	// no triangles exist at the given location
}

void Terrain::setWaterReflectionTex(unsigned texId) {
	pWater_->setReflectionTexture(texId);
}

void Terrain::setWaterRefractionTex(unsigned texId) {
	pWater_->setRefractionTexture(texId);
}

void Terrain::update(float dt) {
	pWater_->update(dt);
}
