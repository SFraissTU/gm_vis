#pragma once
#include <QVector>
#include <QVector3D>
#include <QOpenGLFunctions>
#include <cmath>

namespace gmvis::core {

	struct Helper {

		static void createTriangle(float size, QVector<QVector3D>& vertices, QVector<QVector3D>& normals, QVector<GLuint>& indices) {
			vertices.push_back(QVector3D(0, 0, size));
			vertices.push_back(QVector3D(size, 0, 0));
			vertices.push_back(QVector3D(size, 0, size));
			normals.push_back(QVector3D(0, 0, -1));
			normals.push_back(QVector3D(0, 0, -1));
			normals.push_back(QVector3D(0, 0, -1));
			indices.push_back(0);
			indices.push_back(1);
			indices.push_back(2);
		}

		static void createSphere(float radius, int n, int m, QVector<QVector3D>& vertices, QVector<QVector3D>& normals, QVector<GLuint>& indices) {
			//Taken from my solution in "Introduction to Computer Graphics"
			//n = # vertical segments
			//m = # horizontal segments

			//Top Vertex
			vertices.push_back(QVector3D(0, radius, 0));
			normals.push_back(QVector3D(0, 1, 0));

			float horizontalAngle = 2 * M_PI / m;
			float verticalAngle = M_PI / n;
			//First Row
			for (int j = 0; j < m; j++) {
				float phi = j * horizontalAngle;
				float theta = 1 * verticalAngle;//= first row
				float x = radius * sin(theta) * cos(phi);
				float y = radius * cos(theta);
				float z = radius * sin(theta) * sin(phi);
				QVector3D position(x, y, z);
				vertices.push_back(position);
				normals.push_back(position.normalized());
				if (j > 0) {
					indices.push_back(j + 1);
					indices.push_back(j);
					indices.push_back(0);
				}
			}
			indices.push_back(1);
			indices.push_back(m);
			indices.push_back(0);

			//second to n-1th row
			for (int i = 1; i < n - 1; i++) {
				//Columns
				for (int j = 0; j < m; j++) {
					float phi = j * horizontalAngle;
					float theta = (i + 1) * verticalAngle;
					float x = radius * sin(theta) * cos(phi);
					float y = radius * cos(theta);
					float z = radius * sin(theta) * sin(phi);
					QVector3D position(x, y, z);
					vertices.push_back(position);
					normals.push_back(position.normalized());
					if (j > 0) {
						//Left Top (Last Row, Last Column)
						indices.push_back((i - 1) * m + 1 + j - 1);
						//Right Top (Last Row, This Column)
						indices.push_back((i - 1) * m + 1 + j);
						//Left Bottom (This Row, Last Column)
						indices.push_back(i * m + 1 + j - 1);

						//Left Bottom (This Row, Last Column)
						indices.push_back(i * m + 1 + j - 1);
						//Righ Top (Last Row, This Column)
						indices.push_back((i - 1) * m + 1 + j);
						//Right Bottom (This Row, This Column)
						indices.push_back(i * m + 1 + j);
					}
				}
				//Left Top (Last Row, Last Column)
				indices.push_back((i - 1) * m + 1 + m - 1);
				//Right Top (Last Row, This Column)
				indices.push_back((i - 1) * m + 1);
				//Left Bottom (This Row, Last Column)
				indices.push_back(i * m + 1 + m - 1);

				//Left Bottom (This Row, Last Column)
				indices.push_back(i * m + 1 + m - 1);
				//Righ Top (Last Row, This Column)
				indices.push_back((i - 1) * m + 1);
				//Right Bottom (This Row, This Column)
				indices.push_back(i * m + 1);
			}

			int lastone = (n - 1) * m + 1;
			//Bottom Vertex
			vertices.push_back(QVector3D(0, -radius, 0));
			normals.push_back(QVector3D(0, -1, 0));

			//Last row
			for (int j = 0; j < m - 1; j++) {
				indices.push_back(lastone - m + j + 1);
				indices.push_back(lastone);
				indices.push_back(lastone - m + j);
			}
			indices.push_back(lastone - m);
			indices.push_back(lastone);
			indices.push_back(lastone - 1);
		}

	};
}