#include "plugin.h"
#include "CVector.h"
#include "CCamera.h"
#include "CSprite.h"
#include "CSprite2d.h"
#include "RenderWare.h"

using namespace plugin;

// World and screen constants (update as needed)

class RunwayHud {
public:
    RunwayHud() {
        Events::drawHudEvent += [] {
            // Define the points in the world
            CVector points[] = {
                { 1396.0f, -2494.0103f, 13.0f },
                { 2141.0f, -2494.0103f, 13.0f },
                /*{ 1396.0f, -2629.0f, 0.0f },
                { 2141.0f, -2629.0f, 0.0f },
                { 1396.0f, -2629.0f, 0.0f },*/
            };

            int pointCount = sizeof(points) / sizeof(points[0]);

            // Draw lines between the points
            DrawStaticLines(points, pointCount); // Red lines
            };
    }

    static void DrawStaticLines(const CVector* points, int pointCount) {
        if (pointCount < 2)
            return;

        for (int i = 0; i < pointCount - 1; i++) {
            DrawLine(points[i], points[i + 1]);
        }
    }

    static void DrawLine(CVector start, CVector end) {
        // Determine which points are in front of the camera
        bool startInFront = IsPointInFrontOfCamera(start);
        bool endInFront = IsPointInFrontOfCamera(end);

        // Adjust points if necessary
        if (!startInFront && endInFront) {
            start = GetIntersectionWithNearPlane(start, end);
        }
        else if (startInFront && !endInFront) {
            end = GetIntersectionWithNearPlane(end, start);
        }
        else if (!startInFront && !endInFront) {
            // Both points are behind the camera; don't draw the line
            return;
        }

        RwV3d startRw = { start.x, start.y, start.z };
        RwV3d endRw = { end.x, end.y, end.z };

        RwV3d screenStart, screenEnd;
        float w, h;

        if (CSprite::CalcScreenCoors(startRw, &screenStart, &w, &h, true, true) &&
            CSprite::CalcScreenCoors(endRw, &screenEnd, &w, &h, true, true)) {
            Draw2DLine(screenStart, screenEnd);
        }
    }

    static void Draw2DLine(const RwV3d& start, const RwV3d& end) {
        RwIm2DVertex vertices[2];
        Setup2DVertex(vertices[0], start);
        Setup2DVertex(vertices[1], end);

        // Ensure proper render state
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0); // Disable texture rendering
        RwIm2DRenderLine(vertices, 2, 1, 0); // Render line
    }

    static void Setup2DVertex(RwIm2DVertex& vertex, RwV3d position) {
        vertex.x = position.x;
        vertex.y = position.y;
        //vertex.z = CSprite2d::NearScreenZ;
        vertex.z = position.z;
        vertex.rhw = CSprite2d::RecipNearClip;
        vertex.emissiveColor = RWRGBALONG(255, 0, 0, 0);
        vertex.u = 0.0f;
        vertex.v = 0.0f;
    }
   
    static bool IsPointInFrontOfCamera(const CVector& point) {
        CVector cameraPos = TheCamera.GetPosition();
        CVector cameraForward = TheCamera.m_mCameraMatrix.at; // Camera's forward direction

        // Vector from camera to the point
        CVector toPoint = point - cameraPos;

        // Dot product determines if the point is in front of the camera
        return DotProduct(toPoint, cameraForward) > 0.0f;
    }

    static float DotProduct(const CVector& vec1, const CVector& vec2) {
        return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
    }

    static CVector GetIntersectionWithNearPlane(const CVector& pointBehind, const CVector& pointInFront) {
        CVector cameraPos = TheCamera.GetPosition();
        CVector cameraForward = TheCamera.m_mCameraMatrix.at; // Camera forward vector
        float nearPlaneDistance = 0.1f; // Near clipping plane distance

        // Plane equation: (P - cameraPos) • cameraForward = nearPlaneDistance
        CVector lineDirection = pointInFront - pointBehind;
        float numerator = nearPlaneDistance - DotProduct(pointBehind - cameraPos, cameraForward);
        float denominator = DotProduct(lineDirection, cameraForward);

        // Compute intersection point
        float t = numerator / denominator; // Ratio along the line segment
        return pointBehind + lineDirection * t;
    }
} RunwayHudPlugin;
