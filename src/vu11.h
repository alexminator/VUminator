/*
 * blue red and green dots
 */

void vu11()
{
  uint8_t  i;
  uint16_t minLvlLeft, maxLvlLeft;
  uint16_t minLvlRight, maxLvlRight;
  uint16_t height = auxReading(0);
  int n;

  if (height < 0L)
    height = 0; // Clip output
  else if (height > TOP)
    height = TOP;
  if (height > peakLeft)
    peakLeft = height; // Keep 'peak' dot at top

  // Color pixels based on rainbow gradient
  for (i = 0; i < N_PIXELS; i++)
  {
    if (i >= height)
      strip.setPixelColor(i, 0, 0, 0);
    else
      strip.setPixelColor(i, 0, 0, 255);
  }

  // Draw peak dot
  if (peakLeft > 0 && peakLeft <= N_PIXELS - 1)
    strip.setPixelColor(peakLeft, (map(peakLeft, 0, strip.numPixels() - 1, green, green)));

  strip.show(); // Update strip

  // Every few frames, make the peak pixel drop by 1:

  if (++dotCountLeft >= PEAK_FALL)
  { // fall rate

    if (peakLeft > 0)
      peakLeft--;
    dotCountLeft = 0;
  }

  volLeft[volCountLeft] = n; // Save sample for dynamic leveling
  if (++volCountLeft >= SAMPLES)
    volCountLeft = 0; // Advance/rollover sample counter

  // Get volume range of prior frames
  minLvlLeft = maxLvlLeft = vol[0];
  for (i = 1; i < SAMPLES; i++)
  {
    if (volLeft[i] < minLvlLeft)
      minLvlLeft = volLeft[i];
    else if (volLeft[i] > maxLvlLeft)
      maxLvlLeft = volLeft[i];
  }
  // minLvl and maxLvl indicate the volume range over prior frames, used
  // for vertically scaling the output graph (so it looks interesting
  // regardless of volume level).  If they're too close together though
  // (e.g. at very low volume levels) the graph becomes super coarse
  // and 'jumpy'...so keep some minimum distance between them (this
  // also lets the graph go to zero when no sound is playing):
  if ((maxLvlLeft - minLvlLeft) < TOP)
    maxLvlLeft = minLvlLeft + TOP;
  minLvlAvgLeft = (minLvlAvgLeft * 63 + minLvlLeft) >> 6; // Dampen min/max levels
  maxLvlAvgLeft = (maxLvlAvgLeft * 63 + maxLvlLeft) >> 6; // (fake rolling average)

  n = analogRead(RIGHT_IN_PIN);            // Raw reading from mic
  n = abs(n - 512 - DC_OFFSET);         // Center on zero
  n = (n <= NOISE) ? 0 : (n - NOISE);   // Remove noise/hum
  lvlRight = ((lvlRight * 7) + n) >> 3; // "Dampened" reading (else looks twitchy)

  // Calculate bar height based on dynamic min/max levels (fixed point):
  height = TOP * (lvlRight - minLvlAvgRight) / (long)(maxLvlAvgRight - minLvlAvgRight);

  if (height < 0L)
    height = 0; // Clip output
  else if (height > TOP)
    height = TOP;
  if (height > peakRight)
    peakRight = height; // Keep 'peak' dot at top

  // Color pixels based on rainbow gradient
  for (i = 0; i < N_PIXELS; i++)
  {
    if (i >= height)
      strip1.setPixelColor(i, 0, 0, 0);
    else
      strip1.setPixelColor(i, 0, 0, 255);
  }

  // Draw peak dot
  if (peakRight > 0 && peakRight <= N_PIXELS - 1)
    strip1.setPixelColor(peakRight, (map(peakRight, 0, strip1.numPixels() - 1, red, red)));

  strip1.show(); // Update strip

  // Every few frames, make the peak pixel drop by 1:

  if (++dotCountRight >= PEAK_FALL)
  { // fall rate

    if (peakRight > 0)
      peakRight--;
    dotCountRight = 0;
  }

  volRight[volCountRight] = n; // Save sample for dynamic leveling
  if (++volCountRight >= SAMPLES2)
    volCountRight = 0; // Advance/rollover sample counter

  // Get volume range of prior frames
  minLvlRight = maxLvlRight = vol[0];
  for (i = 1; i < SAMPLES2; i++)
  {
    if (volRight[i] < minLvlRight)
      minLvlRight = volRight[i];
    else if (volRight[i] > maxLvlRight)
      maxLvlRight = volRight[i];
  }
  // minLvl and maxLvl indicate the volume range over prior frames, used
  // for vertically scaling the output graph (so it looks interesting
  // regardless of volume level).  If they're too close together though
  // (e.g. at very low volume levels) the graph becomes super coarse
  // and 'jumpy'...so keep some minimum distance between them (this
  // also lets the graph go to zero when no sound is playing):
  if ((maxLvlRight - minLvlRight) < TOP)
    maxLvlRight = minLvlRight + TOP;
  minLvlAvgRight = (minLvlAvgRight * 63 + minLvlRight) >> 6; // Dampen min/max levels
  maxLvlAvgRight = (maxLvlAvgRight * 63 + maxLvlRight) >> 6; // (fake rolling average)
}