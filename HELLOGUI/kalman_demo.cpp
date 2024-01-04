#include <iostream>
#include <cmath>

class KalmanFilter {
public:
    KalmanFilter(double initialPosition, double initialVelocity, double initialPositionError, double initialVelocityError, double processNoise, double measurementNoise)
        : position(initialPosition), velocity(initialVelocity), positionError(initialPositionError), velocityError(initialVelocityError), processNoise(processNoise), measurementNoise(measurementNoise) {}

    // ����״̬��Э����
    void update(double measuredPosition, double measuredTime) {
        // Ԥ�ⲽ��
        predict(measuredTime);

        // ���²���
        correct(measuredPosition, measuredTime);
    }

    // ��ȡ��ǰ���Ƶ�λ��
    double getPosition() const {
        return position;
    }

    // ��ȡ��ǰ���Ƶ��ٶ�
    double getVelocity() const {
        return velocity;
    }

private:
    double position;  // ��ǰλ�ù���
    double velocity;  // ��ǰ�ٶȹ���
    double positionError; // λ�ù������
    double velocityError; // �ٶȹ������
    double processNoise; // ϵͳ�������� (Q)
    double measurementNoise; // ������������ (R)

    // Ԥ�ⲽ��
    void predict(double deltaTime) {
        // Ԥ��λ�ú��ٶ�
        position += velocity * deltaTime;

        // Ԥ��������ӣ����Ը���ʵ��������е���
        positionError += processNoise;
        velocityError += processNoise;
    }

    // ���²���
    void correct(double measuredPosition, double deltaTime) {
        // ���㿨��������
        double kalmanGain = positionError / (positionError + measurementNoise);

        // ����λ�ú��ٶ�
        position += kalmanGain * (measuredPosition - position);
        velocity += 0.1 * kalmanGain * (measuredPosition - position) / deltaTime; // �˴�ʹ���˻�����ֵ����Ϊ����ٶȵı仯�ܴ�

        // �������
        positionError = (1 - kalmanGain) * positionError;
        velocityError = (1 - kalmanGain) * velocityError;
    }
};