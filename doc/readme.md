* [operations](#operations)
* [shortcuts](#shortcuts)
* [programming](#programming)
* [openmp](#openmp)
* [simd](#simd)

you can write your algorithms and compare the benchmarks between yours and the native algorithms.

## operations
### gestures 
* single finger **`swipe left`**, move the line window to previous dates. the more distance causes the more previous dates.
* single finger **`swipe right`**, move the line window to next dates. the more distance causes the more next dates.
* two fingers **`swipe up`**, zoom in the window, the less dates showed in the window.
* two fingers **`swipe down`**, zoom out the window, the more dates showed in the window.

## shortcuts
### k line view
* **`space`**, highlight one chart of polylines outputed by your aglorithm program. repeat this shortcut key, the next polyline would be chosen.
* **`shift + space`**, highlight one chart of polylines outputed by your aglorithm program, and hide the others.
* **`ctrl + B`**, show the render's benchmarks and statistics info. repeat this shortcut key, the next render policy would be chosen.

### coding editor
* **`ctrl + Q`**, test the program via the default compiler.
* **`alt + Q`**，test the program via an another advanced compiler. this feature needs you download the `resource2-2.pkg`.
* **`ctrl + S`**，save current modified program copy, and the `K Line View` could reload results from your new program.
* **`ctrl + F`**, search the word selection around the current program copy, and highlight all the matches.
* **`F3`**, jump to next matching word searched by **`ctrl + F`**.
* **`ctrl + F3`**, jump to previous matching word searched by **`ctrl + F`**.

## programming
### functions
* basic
  * **`CLOSE()`**, get the sequence of the close prices.
  * **`OPEN()`**, get the sequence of the open prices.
  * **`HIGH()`**, get the sequence of the high prices.
  * **`LOW()`**, get the sequence of the low prices.
  * **`VOL()`**, get the sequence of the Volume.
  * **`DATE()`**, get the sequence of the listed dates.
  * **`REF(S, N)`**, the sequence of data referencing to previous days or next days.
* MA
  * **`MA(S, N)`**, a native implement of MA algorithm.
  * **`SMA(S, N, M)`**, a native implement of SMA algorithm.
  * **`TMA(S, A, B)`**, a native implement of TMA algorithm.
  * **`AMA(S, A)`**, a native implement of AMA algorithm.
  * **`WMA(S, N)`**, a native implement of WMA algorithm.
  * **`EMA(S, N)`**, a native implement of EMA algorithm.
  * **`MEMA(S, N)`**, a native implement of MEMA algorithm.
* other
  * **`HHV(S, N)`**, a native implement of HHV algorithm.
  * **`LLV(S, N)`**, a native implement of LLV algorithm.
  * **`HOD(S, N)`**, a native implement of HOD algorithm.
  * **`LOD(S, N)`**, a native implement of LOD algorithm.
  * **`DEA(S, SHORT, LONG, MID)`**, a native implement of DEA algorithm.
  * **`MACD(S, SHORT, LONG, MID)`**, a native implement of MACD algorithm.
  * **`MAX(S, S2)`**, a native implement of MAX algorithm.
  * **`MIN(S, S2)`**, a native implement of MIN algorithm.
  * **`ABS(S)`**, a native implement of ABS algorithm.
  * **`ABSDIF(S, S2)`**, a native implement of ABSDIF algorithm.
  * **`IF(B, T, F)`**, a native implement of IF algorithm.

### classes
* **`AlgoK`**
* **`AlgoK::Bench`**, help you to benchmark your algorithms, outputs on the `dbgview` tool.
* **`AlgoQ`**

## openmp
OpenMP 4.5 supported.

[example](https://github.com/bbqz007/KTL/blob/master/patch/src/omp-task/AlgoK.cpp)

## simd
SSE3 `tmmintrin.h` supported.
