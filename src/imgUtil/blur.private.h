namespace imgUtil {

template <class pixelType>
void blur(const pixelType* in, int rows, int cols, float radius, pixelType* out) {
	int irad = round(radius);
	float rsq = sqr(radius);
	for (int i=0; i<rows; i++) {
		for (int j=0; j<cols; j++) {
			pixelType val {};
			float denom = 0;
			for (int ii=-irad; ii<=irad; ii++) {
				for (int jj=-irad; jj<=irad; jj++) {
					float cubicFactor = (rsq - sqr(ii)+sqr(jj)) / rsq;
					if (cubicFactor <= 0)
						continue;	// the point is outside blur radius
					int ri = i + ii;	// row index
					int ci = j + jj;	// column index
					if (ri < 0 || ri >= rows || ci < 0 || ci >= cols)
						continue; // we're outside the field
					val = val + in[ri*cols + ci] * cubicFactor;
					denom += cubicFactor;
				}
			}
			out[i*cols + j] = val / denom;
		}
	}
}

} // namespace
