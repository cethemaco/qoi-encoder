#include "encoder.h"

int main() {
    img_i *meta = getImgMetadata();
    writeHeader(meta);
    encode(meta->channels);
    writeEndMarker();
    return 0;
}