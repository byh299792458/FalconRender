#ifndef __MODEL_HPP__
#define __MODEL_HPP__

#include <exception>
#include <sstream>
#include <string>
#include <vector>

#include <Eigen/Eigen>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace flr 
{
	using vec4f = Eigen::Vector4f;
	using vec3f = Eigen::Vector3f;
	using vec2f = Eigen::Vector2f;

	struct Vertex {
		vec4f position;
		vec3f normal;
		std::vector<vec2f> texcoords;
	};

	struct Face {
		std::vector<int> indices;
	};

	struct Mesh {
		std::vector<Vertex> vertices_;
		std::vector<Face> faces_;
	};

	class Model {
	public:
		std::vector<Mesh> meshes_;
		std::vector<Vertex> vertex_buffer_obj_;
		std::vector<int> element_buffer_obj_;

		void LoadModel(std::string path) {
			Assimp::Importer importer;
			/*
				aiProcess_Triangulate: ���ģ�Ͳ��ǻ�ȫ������������ɣ���Ҫ��ģ�����е�ͼԪ��״�任Ϊ������
				aiProcess_FlipUVs: ��תy�����������
				aiProcess_GenNormals: ���ģ�Ͳ���������������Ϊÿ�����㴴��������
				aiProcess_SplitLargeMeshes: ���Ƚϴ������ָ�ɸ�С�������������Ⱦ����󶥵������ƣ�ֻ����Ⱦ��С���������ѡ��������ã�
				aiProcess_OptimizeMeshes: �����С����ƴ��Ϊ�����񣬼��ٻ��Ƶ��ôӶ������Ż�����aiProcess_SplitLargeMeshes�෴��
			*/
			const aiScene* scene = importer.ReadFile(path,
				aiProcess_Triangulate | aiProcess_FlipUVs);
			if (!scene
				|| scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE
				|| !scene->mRootNode) {
				std::stringstream ss;
				ss << "error::assimp::" << __FILE__ << "::" << __LINE__ << "::" << importer.GetErrorString() << '\n';
				throw std::exception(ss.str().data());
			}
			ProcessNode(scene->mRootNode, scene);
		}
		void ProcessNode(aiNode* node, const aiScene* scene)
		{
			for (int i = 0; i < node->mNumMeshes; ++i) {
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				ProcessMesh(mesh, scene);
			}
			for (int i = 0; i < node->mNumChildren; ++i)
			{
				ProcessNode(node->mChildren[i], scene);
			}
		}
		void ProcessMesh(aiMesh* mesh, const aiScene* scene)
		{
			meshes_.push_back(Mesh());
			auto& last = meshes_.back();

			for (int i = 0; i < mesh->mNumVertices; ++i) {
				Vertex vertex;
				vertex.position[0] = mesh->mVertices[i].x;
				vertex.position[1] = mesh->mVertices[i].y;
				vertex.position[2] = mesh->mVertices[i].z;
				vertex.position[3] = 1;
				vertex.normal[0] = mesh->mNormals[i].x;
				vertex.normal[1] = mesh->mNormals[i].y;
				vertex.normal[2] = mesh->mNormals[i].z;
				for (int j = 0; mesh->mTextureCoords[j]; ++j) {
					vertex.texcoords.emplace_back(
						mesh->mTextureCoords[j][i].x,
						mesh->mTextureCoords[j][i].y
					);
				}
				last.vertices_.push_back(vertex);
				vertex_buffer_obj_.push_back(vertex);
			}

			for (int i = 0; i < mesh->mNumFaces; ++i) {
				aiFace face = mesh->mFaces[i];
				Face f;
				for (int j = 0; j < face.mNumIndices; ++j)
				{
					f.indices.push_back(face.mIndices[j]);
					element_buffer_obj_.push_back(face.mIndices[j]);
				}
				last.faces_.push_back(std::move(f));
			}
		}
	};
}

#endif // !__MODEL_HPP__
