///////////////////////////////////////////////////////////////////////////////
// Fast 2D Convolution using integer multiplication instead of float.
// Multiply coefficient(factor) to accumulated sum at last.
// NOTE: IT IS NOT FASTER THAN FLOAT MULTIPLICATION, TRY YOURSELF!!!
///////////////////////////////////////////////////////////////////////////////
bool convolve2DFast2(unsigned char* in, unsigned char* out, int dataSizeX, int dataSizeY,
                    int* kernel, float factor, int kernelSizeX, int kernelSizeY)
{
    int i, j, m, n, x, y, t;
    unsigned char **inPtr, *outPtr, *ptr;
    int kCenterX, kCenterY;
    int rowEnd, colEnd;                             // ending indice for section divider
    int sum;                                        // temp accumulation buffer
    int k, kSize;

    // check validity of params
    if(!in || !out || !kernel) return false;
    if(dataSizeX <= 0 || kernelSizeX <= 0) return false;

    // find center position of kernel (half of kernel size)
    kCenterX = kernelSizeX >> 1;
    kCenterY = kernelSizeY >> 1;
    kSize = kernelSizeX * kernelSizeY;              // total kernel size

    // allocate memeory for multi-cursor
    inPtr = new unsigned char*[kSize];
    if(!inPtr) return false;                        // allocation error

    // set initial position of multi-cursor, NOTE: it is swapped instead of kernel
    ptr = in + (dataSizeX * kCenterY + kCenterX); // the first cursor is shifted (kCenterX, kCenterY)
    for(m=0, t=0; m < kernelSizeY; ++m)
    {
        for(n=0; n < kernelSizeX; ++n, ++t)
        {
            inPtr[t] = ptr - n;
        }
        ptr -= dataSizeX;
    }

    // init working  pointers
    outPtr = out;

    rowEnd = dataSizeY - kCenterY;                  // bottom row partition divider
    colEnd = dataSizeX - kCenterX;                  // right column partition divider

    // convolve rows from index=0 to index=kCenterY-1
    y = kCenterY;
    for(i=0; i < kCenterY; ++i)
    {
        // partition #1 ***********************************
        x = kCenterX;
        for(j=0; j < kCenterX; ++j)                 // column from index=0 to index=kCenterX-1
        {
            sum = 0;
            t = 0;
            for(m=0; m <= y; ++m)
            {
                for(n=0; n <= x; ++n)
                {
                    sum += *inPtr[t] * kernel[t];
                    ++t;
                }
                t += (kernelSizeX - x - 1);         // jump to next row
            }

            // store output
            *outPtr = (unsigned char)(fabs(sum * factor) + 0.5f);
            ++outPtr;
            ++x;
            for(k=0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
        }

        // partition #2 ***********************************
        for(j=kCenterX; j < colEnd; ++j)            // column from index=kCenterX to index=(dataSizeX-kCenterX-1)
        {
            sum = 0;
            t = 0;
            for(m=0; m <= y; ++m)
            {
                for(n=0; n < kernelSizeX; ++n)
                {
                    sum += *inPtr[t] * kernel[t];
                    ++t;
                }
            }

            // store output
            *outPtr = (unsigned char)(fabs(sum * factor) + 0.5f);
            ++outPtr;
            ++x;
            for(k=0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
        }

        // partition #3 ***********************************
        x = 1;
        for(j=colEnd; j < dataSizeX; ++j)           // column from index=(dataSizeX-kCenter) to index=(dataSizeX-1)
        {
            sum = 0;
            t = x;
            for(m=0; m <= y; ++m)
            {
                for(n=x; n < kernelSizeX; ++n)
                {
                    sum += *inPtr[t] * kernel[t];
                    ++t;
                }
                t += x;                             // jump to next row
            }

            // store output
            *outPtr = (unsigned char)(fabs(sum * factor) + 0.5f);
            ++outPtr;
            ++x;
            for(k=0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
        }

        ++y;                                        // add one more row to convolve for next run
    }

    // convolve rows from index=kCenterY to index=(dataSizeY-kCenterY-1)
    for(i= kCenterY; i < rowEnd; ++i)               // number of rows
    {
        // partition #4 ***********************************
        x = kCenterX;
        for(j=0; j < kCenterX; ++j)                 // column from index=0 to index=kCenterX-1
        {
            sum = 0;
            t = 0;
            for(m=0; m < kernelSizeY; ++m)
            {
                for(n=0; n <= x; ++n)
                {
                    sum += *inPtr[t] * kernel[t];
                    ++t;
                }
                t += (kernelSizeX - x - 1);
            }

            // store output
            *outPtr = (unsigned char)(fabs(sum * factor) + 0.5f);
            ++outPtr;
            ++x;
            for(k=0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
        }

        // partition #5 ***********************************
        for(j = kCenterX; j < colEnd; ++j)          // column from index=kCenterX to index=(dataSizeX-kCenterX-1)
        {
            sum = 0;
            t = 0;
            for(m=0; m < kernelSizeY; ++m)
            {
                for(n=0; n < kernelSizeX; ++n)
                {
                    sum += *inPtr[t] * kernel[t];
                    ++inPtr[t]; // in this partition, all cursors are used to convolve. moving cursors to next is safe here
                    ++t;
                }
            }

            // store output
            *outPtr = (unsigned char)(fabs(sum * factor) + 0.5f);
            ++outPtr;
            ++x;
        }

        // partition #6 ***********************************
        x = 1;
        for(j=colEnd; j < dataSizeX; ++j)           // column from index=(dataSizeX-kCenter) to index=(dataSizeX-1)
        {
            sum = 0;
            t = x;
            for(m=0; m < kernelSizeY; ++m)
            {
                for(n=x; n < kernelSizeX; ++n)
                {
                    sum += *inPtr[t] * kernel[t];
                    ++t;
                }
                t += x;
            }

            // store output
            *outPtr = (unsigned char)(fabs(sum * factor) + 0.5f);
            ++outPtr;
            ++x;
            for(k=0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
        }
    }

    // convolve rows from index=(dataSizeY-kCenterY) to index=(dataSizeY-1)
    y = 1;
    for(i= rowEnd; i < dataSizeY; ++i)               // number of rows
    {
        // partition #7 ***********************************
        x = kCenterX;
        for(j=0; j < kCenterX; ++j)                 // column from index=0 to index=kCenterX-1
        {
            sum = 0;
            t = kernelSizeX * y;

            for(m=y; m < kernelSizeY; ++m)
            {
                for(n=0; n <= x; ++n)
                {
                    sum += *inPtr[t] * kernel[t];
                    ++t;
                }
                t += (kernelSizeX - x - 1);
            }

            // store output
            *outPtr = (unsigned char)(fabs(sum * factor) + 0.5f);
            ++outPtr;
            ++x;
            for(k=0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
        }

        // partition #8 ***********************************
        for(j=kCenterX; j < colEnd; ++j)            // column from index=kCenterX to index=(dataSizeX-kCenterX-1)
        {
            sum = 0;
            t = kernelSizeX * y;
            for(m=y; m < kernelSizeY; ++m)
            {
                for(n=0; n < kernelSizeX; ++n)
                {
                    sum += *inPtr[t] * kernel[t];
                    ++t;
                }
            }

            // store output
            *outPtr = (unsigned char)(fabs(sum * factor) + 0.5f);
            ++outPtr;
            ++x;
            for(k=0; k < kSize; ++k) ++inPtr[k];
        }

        // partition #9 ***********************************
        x = 1;
        for(j=colEnd; j < dataSizeX; ++j)           // column from index=(dataSizeX-kCenter) to index=(dataSizeX-1)
        {
            sum = 0;
            t = kernelSizeX * y + x;
            for(m=y; m < kernelSizeY; ++m)
            {
                for(n=x; n < kernelSizeX; ++n)
                {
                    sum += *inPtr[t] * kernel[t];
                    ++t;
                }
                t += x;
            }

            // store output
            *outPtr = (unsigned char)(fabs(sum * factor) + 0.5f);
            ++outPtr;
            ++x;
            for(k=0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
        }

        ++y;                                        // the starting row index is increased
    }

    return true;
}
