# test
some test project

# qtFeatureBgsTest
a test project use Qt and opencv:
- 1. background substraction(BGS)
- 2. local binary pattern(LBP)
- 3. feature tracking

# small trick
- mix the FG generated by separated BGS algorithm
- split "process()" into "processWithoutUpdate()" and "updateWithMovingStatic()"
- remove the keypoint out of dilated "mixed raw FG", so we need to remember when match the feature point between previous img and current img, the previous keypoint is not enough.
- decide when to accept the foreground generate by npe_bgs. (usually, the npe_bgs is best), but sometimes the npe_bgs is polluted by light change. 
- light change pollution check: bigest area > 0.1 * area of total img, "polluted FG area" >> "sum of other FG area".
- median filter, morphlogyEX, imfill
- lbp, lab(better than hsv)
- two lever lru cache replace strategy
- HOW about this? matched keypoint as normal, unmatched keypoint as unknow. then for an area, if no keyPoint matched, removed it!
- TODO: abnormal match check: the moving speed check. for example, distance > L2(img_cols/k,img_rows/k);

# code trick
- use CV_Assert().
- use yzbx_frameInput class to get frame from camera/video file/frame folder
- use yzbx_distance() function to compute L1/L2/hamming distance
- use yzbx_config to remember the config.