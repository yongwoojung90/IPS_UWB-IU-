

float KalmanFilter(int AnchorNumber, float distance, float Q = 0.0071f, float R = 0.608f)
{
	static float xhat[4] = { 0.0f, };
	static float P[4] = { 0.0f, };
	static float xhatbar[4] = { 0.0f, };
	static float Pbar[4] = { 0.0f, };
	static float K[4] = { 0.0f, };

	static float xhat_last[4] = { 0.0f, };
	static float P_last[4] = { 1, 1, 1, 1 };

	xhatbar[AnchorNumber] = xhat_last[AnchorNumber];
	Pbar[AnchorNumber] = P_last[AnchorNumber] + Q;

	K[AnchorNumber] = Pbar[AnchorNumber] / (Pbar[AnchorNumber] + R);
	xhat[AnchorNumber] = xhatbar[AnchorNumber] + (K[AnchorNumber] * (distance - xhatbar[AnchorNumber]));
	P[AnchorNumber] = (1 - K[AnchorNumber]) * Pbar[AnchorNumber];

	xhat_last[AnchorNumber] = xhat[AnchorNumber];
	P_last[AnchorNumber] = P[AnchorNumber];

	return xhat[AnchorNumber];
}