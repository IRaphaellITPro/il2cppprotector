#include "Rect.h"
#include "Quaternion.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include "MonoString.h"
#include "Color.h"

using namespace std;

struct LayerMask {
private:
    int mask;
    
public:
    LayerMask() {
        this->mask = 0;
    }
    int get_value() {
        return this->mask;
    }
    void set_value(int value) {
        this->mask = value;
    }
};

struct RaycastHit {
private:
    Vector3 m_Point;
    Vector3 m_Normal;
    uint32_t m_FaceID;
    float m_Distance;
    Vector2 m_UV;
    int m_Collider;
    
public:
    Vector3 getPoint() {
        return this->m_Point;
    }
    Vector3 getNormal() {
        return this->m_Normal;
    }
    uint32_t getFaceID() {
        return this->m_FaceID;
    }
    float getDistance() {
        return this->m_Distance;
    }
    Vector2 getUV() {
        return this->m_UV;
    }
    int getColliderID() {
        return this->m_Collider;
    }
    std::string toString() {
        std::string result;
        result = "Point {" + Vector3::ToString(this->m_Point) + "}\n";
        result += "Normal {" + Vector3::ToString(this->m_Normal) + "}\n";
        result += "FaceID {" + to_string(this->m_FaceID) + "}\n";
        result += "Distance {" + to_string(this->m_Distance) + "}\n";
        result += "Collider ID {" + to_string(this->m_Collider) + "}";
        return result;
    }
};

struct Ray {
private:
    Vector3 m_Origin;
    Vector3 m_Direction;
	
public:
    Ray(Vector3 original, Vector3 direction) {
        this->m_Origin = original;
        this->m_Direction = direction;
    }
	Vector3 get_Origin() {
		return this->m_Origin;
	}
	Vector3 get_Direction() {
		return this->m_Direction;
	}
	void set_Origin(Vector3 value) {
		this->m_Origin = value;
	}
	void set_Direction(Vector3 value) {
		this->m_Direction = value;
	}
	Vector3 GetPoint(float distance) {
		return this->m_Origin + this->m_Direction * distance;
	}
	string ToString() {
		string text = "";
		text += "Origin(";
		text += to_string(this->m_Origin.x) + ", ";
		text += to_string(this->m_Origin.y) + ", ";
		text += to_string(this->m_Origin.z) + ")\n";
		text += "Direction(";
		text += to_string(this->m_Direction.x) + ", ";
		text += to_string(this->m_Direction.y) + ", ";
		text += to_string(this->m_Direction.z) + ")";
		return text;
	}
};

enum TouchPhase {
	Began = 0,
	Moved = 1,
	Stationary = 2,
	Ended = 3,
	Canceled = 4,
};
enum TouchType {
	Direct = 0,
	Indirect = 1,
	Stylus = 2,
};

struct Touch {
	int m_FingerId;
	Vector2 m_Position;
	Vector2 m_RawPosition;
	Vector2 m_PositionDelta;
	float m_TimeDelta;
	int m_TapCount;
	TouchPhase m_Phase;
	TouchType m_Type;
	float m_Pressure;
	float m_maximumPossiblePressure;
	float m_Radius;
	float m_RadiusVariance;
	float m_AltitudeAngle;
	float m_AzimuthAngle;
};