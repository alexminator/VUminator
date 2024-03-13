// ------------------
// -- VU functions --
// ------------------

uint16_t auxReading(uint8_t channel) {

  uint16_t height = 0;

  if(channel == 0) {
    int n = analogRead(LEFT_IN_PIN); // Raw reading from left line in
    n = abs(n - 512 - DC_OFFSET); // Center on zero
    n = (n <= NOISE) ? 0 : (n - NOISE); // Remove noise/hum
    lvlLeft = ((lvlLeft * 15) + n) >> 4; // "Dampened" reading else looks twitchy (>>3 is divide by 32)
    volLeft[volCountLeft] = n * ((float)(1023 - sensitivity) / 512);
    volCountLeft = (volCountLeft + 1) % SAMPLES;
    // Calculate bar height based on dynamic min/max levels (fixed point):
    height = TOP * (lvlLeft - minLvlAvgLeft) / (long)(maxLvlAvgLeft - minLvlAvgLeft);
  }
  
  else {
    int n = analogRead(RIGHT_IN_PIN); // Raw reading from mic
    n = abs(n - 512 - DC_OFFSET); // Center on zero
    n = (n <= NOISE) ? 0 : (n - NOISE); // Remove noise/hum
    lvlRight = ((lvlRight * 15) + n) >> 4; // "Dampened" reading (else looks twitchy)
    volRight[volCountRight] = n * ((float)(1023 - sensitivity) / 512); // Save sample for dynamic leveling
    volCountRight = (volCountRight +1 ) % SAMPLES;
    // Calculate bar height based on dynamic min/max levels (fixed point):
    height = TOP * (lvlRight - minLvlAvgRight) / (long)(maxLvlAvgRight - minLvlAvgRight);
  }
  height = fmin(height, TOP);
  return height;
}

void copyLeftToRight() {
  for (uint8_t i = 0; i < N_PIXELS; i++) {
    ledsRight[i] = ledsLeft[i];
  }
}

/*
 * Function for dropping the peak
 */
uint8_t peakLeft, peakRight;
void dropPeak(uint8_t channel) {
  
  static uint8_t dotCountLeft, dotCountRight;
 
  if(channel == 0) {
    if(++dotCountLeft >= PEAK_FALL) { //fall rate 
      if(peakLeft > 0) peakLeft--;
      dotCountLeft = 0;
    }
  } else {
    if(++dotCountRight >= PEAK_FALL) { //fall rate 
      if(peakRight > 0) peakRight--;
      dotCountRight = 0;
    }
  }
}

/*
 * Function for averaging the sample readings
 */
void averageReadings(uint8_t channel) {

  uint16_t minLvl, maxLvl;

  // minLvl and maxLvl indicate the volume range over prior frames, used
  // for vertically scaling the output graph (so it looks interesting
  // regardless of volume level).  If they're too close together though
  // (e.g. at very low volume levels) the graph becomes super coarse
  // and 'jumpy'...so keep some minimum distance between them (this
  // also lets the graph go to zero when no sound is playing):
  if(channel == 0) {
    minLvl = maxLvl = volLeft[0];
    for (int i = 1; i < SAMPLES; i++) {
      if (volLeft[i] < minLvl) minLvl = volLeft[i];
      else if (volLeft[i] > maxLvl) maxLvl = volLeft[i];
    }
    if ((maxLvl - minLvl) < TOP) maxLvl = 10;
    
    minLvlAvgLeft = (minLvlAvgLeft * 63 + minLvl) >> 6; // Dampen min/max levels
    maxLvlAvgLeft = (maxLvlAvgLeft * 63 + maxLvl) >> 6; // (fake rolling average)
  }

  else {
    minLvl = maxLvl = volRight[0];
    for (int i = 1; i < SAMPLES; i++) {
      if (volRight[i] < minLvl) minLvl = volRight[i];
      else if (volRight[i] > maxLvl) maxLvl = volRight[i];
    }
    if ((maxLvl - minLvl) < TOP) maxLvl = 10;
    minLvlAvgRight = (minLvlAvgRight * 63 + minLvl) >> 6; // Dampen min/max levels
    maxLvlAvgRight = (maxLvlAvgRight * 63 + maxLvl) >> 6; // (fake rolling average)
  }
}