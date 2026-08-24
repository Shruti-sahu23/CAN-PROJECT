#include "distance_sensor.h"
#include "delays.h"

static f32 filtered_distance = 100.0f;
static u8 last_stable_distance = 100;
static u8 is_first_run = 1;
 
void Distance_Sensor_Init(void)
{
    ADC_Init();
    filtered_distance = 100.0f;
    last_stable_distance = 100;
    is_first_run = 1;
}

/*
 * Collects 20 samples, sorts them to reject outliers,
 * and computes the trimmed average of the middle 8 values.
 */
static u16 ADC_Sample_Filtered(void)
{
    u16 samples[DIST_SAMPLE_COUNT];
    u32 sum = 0;
    int i, j;
    u16 temp;
    
    /* 1. Collect 20 successive ADC samples */
    for(i = 0; i < DIST_SAMPLE_COUNT; i++)
    {
        samples[i] = ADC_Read();
        delay_us(150);
    }
    
    /* 2. Sort samples in ascending order */
    for(i = 0; i < DIST_SAMPLE_COUNT - 1; i++)
    {
        for(j = 0; j < DIST_SAMPLE_COUNT - i - 1; j++)
        {
            if(samples[j] > samples[j + 1])
            {
                temp = samples[j];
                samples[j] = samples[j + 1];
                samples[j + 1] = temp;
            }
        }
    }
    
    /* 3. Trimmed Mean: Average the middle 8 values (indices 6 to 13) */
    for(i = 6; i <= 13; i++)
    {
        sum += samples[i];
    }
    
    return (u16)(sum / 8);
}

u8 Distance_Calculate_cm(void)
{
    u16 adc_avg;
    f32 raw_dist;
    
    adc_avg = ADC_Sample_Filtered();
    
    /* Sharp GP2D12 Inverse Voltage-Distance Conversion */
    if(adc_avg <= 80)
    {
        raw_dist = 120.0f; /* Safe / Out of near range */
    }
    else
    {
        raw_dist = (6787.0f / (f32)(adc_avg - 3)) - 4.0f;
    }
    
    /* Clamp operating limits (10 cm to 120 cm) */
    if(raw_dist < 10.0f)  raw_dist = 10.0f;
    if(raw_dist > 120.0f) raw_dist = 120.0f;
    
    if(is_first_run)
    {
        filtered_distance = raw_dist;
        last_stable_distance = (u8)(raw_dist + 0.5f);
        is_first_run = 0;
        return last_stable_distance;
    }
    
    /* 4. Heavy Digital Low-Pass Filter (alpha = 0.15 for high stability) */
    filtered_distance = (0.15f * raw_dist) + (0.85f * filtered_distance);
    
    /* 5. 2.0 cm Schmitt-Trigger Deadband Hysteresis */
    if(((f32)last_stable_distance - filtered_distance) >= DIST_DEADBAND_CM ||
       (filtered_distance - (f32)last_stable_distance) >= DIST_DEADBAND_CM)
    {
        last_stable_distance = (u8)(filtered_distance + 0.5f);
    }
    
    return last_stable_distance;
}

u8 Distance_Determine_Zone(u8 distance_cm)
{
    if(distance_cm >= 80)
    {
        return ZONE_SAFE;      /* >= 80 cm: Safe Zone */
    }
    else if(distance_cm >= 40)
    {
        return ZONE_WARNING;   /* 40 - 79 cm: Warning Zone */
    }
    else if(distance_cm >= 20)
    {
        return ZONE_DANGER;    /* 20 - 39 cm: Danger Zone */
    }
    else
    {
        return ZONE_CRITICAL;  /* < 20 cm: Critical Stop */
    }
}
