// ###########################################################################
//          Title: Skibuino Sounds
//         Author: Mike Del Pozzo
//    Description: Sounds for Skibuino.
//        Version: 1.0.0
//           Date: 19 Mar 2018
//        License: GPLv3 (see LICENSE)
//
//    Skibuino Copyright (C) 2018 Mike Del Pozzo
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    any later version.
// ###########################################################################


const int sndJump[10] =
{
  4,   // 0: Volume
  0,   // 1: Instrument ID
  1,   // 2: Volume Slide Step Duration
  -1,  // 3: Volume Slide Step Depth
  1,   // 4: Pitch Slide Step Duration
  2,   // 5: Pitch Slide Step Depth
  0,   // 6: Tremolo Step Duration
  0,   // 7: Tremolo Step Depth
  23,  // 8: Pitch
  4    // 9: Duration
};

const int sndSlide[10] =
{
  4,   // 0: Volume
  1,   // 1: Instrument ID
  1,   // 2: Volume Slide Step Duration
  -1,  // 3: Volume Slide Step Depth
  1,   // 4: Pitch Slide Step Duration
  -2,  // 5: Pitch Slide Step Depth
  0,   // 6: Tremolo Step Duration
  0,   // 7: Tremolo Step Depth
  16,  // 8: Pitch
  4    // 9: Duration
};

const int sndCrash[10] =
{
  6,   // 0: Volume
  1,   // 1: Instrument ID
  1,   // 2: Volume Slide Step Duration
  -1,  // 3: Volume Slide Step Depth
  0,   // 4: Pitch Slide Step Duration
  0,   // 5: Pitch Slide Step Depth
  0,   // 6: Tremolo Step Duration
  0,   // 7: Tremolo Step Depth
  0,   // 8: Pitch
  6    // 9: Duration
};

const int sndResume[10] =
{
  1,   // 0: Volume
  0,   // 1: Instrument ID
  0,   // 2: Volume Slide Step Duration
  0,   // 3: Volume Slide Step Depth
  4,   // 4: Pitch Slide Step Duration
  -5,  // 5: Pitch Slide Step Depth
  0,   // 6: Tremolo Step Duration
  0,   // 7: Tremolo Step Depth
  60,  // 8: Pitch
  6    // 9: Duration
};

const int sndPause[10] =
{
  1,   // 0: Volume
  0,   // 1: Instrument ID
  0,   // 2: Volume Slide Step Duration
  0,   // 3: Volume Slide Step Depth
  4,   // 4: Pitch Slide Step Duration
  5,   // 5: Pitch Slide Step Depth
  0,   // 6: Tremolo Step Duration
  0,   // 7: Tremolo Step Depth
  55,  // 8: Pitch
  6    // 9: Duration
};

