#include <iostream>
#include <cmath>

class KalmanFilter {
public:
    KalmanFilter(double initialPosition, double initialVelocity, double initialPositionError, double initialVelocityError, double processNoise, double measurementNoise)
        : position(initialPosition), velocity(initialVelocity), positionError(initialPositionError), velocityError(initialVelocityError), processNoise(processNoise), measurementNoise(measurementNoise) {}

    // 更新状态和协方差
    void update(double measuredPosition, double measuredTime) {
        // 预测步骤
        predict(measuredTime);

        // 更新步骤
        correct(measuredPosition, measuredTime);
    }

    // 获取当前估计的位置
    double getPosition() const {
        return position;
    }

    // 获取当前估计的速度
    double getVelocity() const {
        return velocity;
    }

private:
    double position;  // 当前位置估计
    double velocity;  // 当前速度估计
    double positionError; // 位置估计误差
    double velocityError; // 速度估计误差
    double processNoise; // 系统噪声方差 (Q)
    double measurementNoise; // 测量噪声方差 (R)

    // 预测步骤
    void predict(double deltaTime) {
        // 预测位置和速度
        position += velocity * deltaTime;

        // 预测误差增加，可以根据实际情况进行调整
        positionError += processNoise;
        velocityError += processNoise;
    }

    // 更新步骤
    void correct(double measuredPosition, double deltaTime) {
        // 计算卡尔曼增益
        double kalmanGain = positionError / (positionError + measurementNoise);

        // 更新位置和速度
        position += kalmanGain * (measuredPosition - position);
        velocity += 0.1 * kalmanGain * (measuredPosition - position) / deltaTime; // 此处使用了滑动均值，因为鼠标速度的变化很大

        // 更新误差
        positionError = (1 - kalmanGain) * positionError;
        velocityError = (1 - kalmanGain) * velocityError;
    }
};