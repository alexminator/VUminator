/*
 * shooting star
 */

void vu12()
{
  uint16_t height = auxReading(0);

  if (height < 0L)
    height = 0; // Clip output
  else if (height > TOP)
    height = TOP;
  if (height > peakLeft)
    peakLeft = height; // Keep 'peak' dot at top

#ifdef CENTERED
  // Color pixels based on rainbow gradient
  for (i = 0; i < (N_PIXELS / 2); i++)
  {
    if (((N_PIXELS / 2) + i) >= height)
    {
      strip.setPixelColor(((N_PIXELS / 2) + i), 0, 0, 0);
      strip.setPixelColor(((N_PIXELS / 2) - i), 0, 0, 0);
    }
    else
    {
      strip.setPixelColor(((N_PIXELS / 2) + i), Wheel(map(((N_PIXELS / 2) + i), 0, strip.numPixels() - 1, 30, 150)));
      strip.setPixelColor(((N_PIXELS / 2) - i), Wheel(map(((N_PIXELS / 2) - i), 0, strip.numPixels() - 1, 30, 150)));
    }
  }

  // Draw peak dot
  if (peakLeft > 0 && peakRight <= LAST_PIXEL_OFFSET)
  {
    strip.setPixelColor(((N_PIXELS / 2) + peakLeft), 255, 255, 255); // (peak,Wheel(map(peak,0,strip.numPixels()-1,30,150)));
    strip.setPixelColor(((N_PIXELS / 2) - peakLeft), 255, 255, 255); // (peak,Wheel(map(peak,0,strip.numPixels()-1,30,150)));
  }
#else
  // Color pixels based on rainbow gradient
  for (i = 0; i < N_PIXELS; i++)
  {
    if (i >= height)
    {
      strip.setPixelColor(i, 0, 0, 0);
    }
    else
    {
      strip.setPixelColor(i, Wheel(map(i, 0, strip.numPixels() - 1, 30, 150)));
    }
  }

  // Draw peak dot
  if (peakLeft > 0 && peakLeft <= LAST_PIXEL_OFFSET)
  {
    strip.setPixelColor(peakLeft, 255, 255, 255); // (peak,Wheel(map(peak,0,strip.numPixels()-1,30,150)));
  }

#endif

  // Every few frames, make the peak pixel drop by 1:

  if (millis() - lastTime >= PEAK_FALL_MILLIS)
  {
    lastTime = millis();

    strip.show(); // Update strip

    // fall rate
    if (peakLeft > 0)
      peakLeft--;
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

  n = analogRead(RIGHT_IN_PIN);         // Raw reading from mic
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

#ifdef CENTERED
  // Color pixels based on rainbow gradient
  for (i = 0; i < (N_PIXELS / 2); i++)
  {
    if (((N_PIXELS / 2) + i) >= height)
    {
      strip1.setPixelColor(((N_PIXELS / 2) + i), 0, 0, 0);
      strip1.setPixelColor(((N_PIXELS / 2) - i), 0, 0, 0);
    }
    else
    {
      strip1.setPixelColor(((N_PIXELS / 2) + i), Wheel(map(((N_PIXELS / 2) + i), 0, strip1.numPixels() - 1, 30, 150)));
      strip1.setPixelColor(((N_PIXELS / 2) - i), Wheel(map(((N_PIXELS / 2) - i), 0, strip1.numPixels() - 1, 30, 150)));
    }
  }

  // Draw peak dot
  if (peakRight > 0 && peakRight <= LAST_PIXEL_OFFSET)
  {
    strip1.setPixelColor(((N_PIXELS / 2) + peakRight), 255, 255, 255); // (peak,Wheel(map(peak,0,strip.numPixels()-1,30,150)));
    strip1.setPixelColor(((N_PIXELS / 2) - peakRight), 255, 255, 255); // (peak,Wheel(map(peak,0,strip.numPixels()-1,30,150)));
  }
#else
  // Color pixels based on rainbow gradient
  for (i = 0; i < N_PIXELS; i++)
  {
    if (i >= height)
    {
      strip1.setPixelColor(i, 0, 0, 0);
    }
    else
    {
      strip1.setPixelColor(i, Wheel(map(i, 0, strip1.numPixels() - 1, 30, 150)));
    }
  }

  // Draw peak dot
  if (peakRight > 0 && peakRight <= LAST_PIXEL_OFFSET)
  {
    strip1.setPixelColor(peakRight, 255, 255, 255); // (peak,Wheel(map(peak,0,strip.numPixels()-1,30,150)));
  }

#endif

  // Every few frames, make the peak pixel drop by 1:

  if (millis() - lastTime >= PEAK_FALL_MILLIS)
  {
    lastTime = millis();

    strip1.show(); // Update strip

    // fall rate
    if (peakRight > 0)
      peakRight--;
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