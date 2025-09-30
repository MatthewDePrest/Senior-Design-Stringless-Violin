import React, { useState } from 'react';
import { Slider, Typography, Button, Box } from '@mui/material';

const STRING_NAMES = ['First', 'Second', 'Third', 'Fourth'];

function Tuner({ onSave }) {
    // Default standard violin tuning frequencies in Hz
    const violinDefault = {
        First: 196.00,
        Second: 293.00,
        Third: 440.00,
        Fourth: 659.00,
    };

    // Default standard viola tuning frequencies in Hz
    const violaDefault = {
        First: 131.00,
        Second: 196.00,
        Third: 293.00,
        Fourth: 440.00,
    };

    const [frequencies, setFrequencies] = useState(violinDefault);

    const handleSliderChange = (string) => (event, newValue) => {
        setFrequencies((prev) => ({ ...prev, [string]: newValue }));
    };

    const setDefaultViolin = () => {
        setFrequencies(violinDefault);
    };

    const setDefaultViola = () => {
        setFrequencies(violaDefault);
    };

    const saveToFile = async () => {
    try {
        const response = await fetch('http://localhost:5000/save-frequencies', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(frequencies),
        });
        if (response.ok) {
            alert('Frequencies saved successfully!');
        } else {
            alert('Failed to save frequencies.');
        }
    } catch (error) {
        alert('Error connecting to server.');
        console.error(error);
    }
    };



    return (
        <Box sx={{ width: 300, margin: 'auto', mt: 4 }}>
        <Typography variant="h5" gutterBottom>Tuner Settings</Typography>
        {STRING_NAMES.map((string) => (
            <Box key={string} sx={{ mb: 3 }}>
                <Typography gutterBottom>{string} String: {frequencies[string].toFixed(0)} Hz</Typography>
                <Slider
                    value={frequencies[string]}
                    onChange={handleSliderChange(string)}
                    min={50}
                    max={1000}
                    step={1}
                    aria-labelledby={`${string}-string-slider`}
                />
            </Box>
        ))}
      
        <Button variant="outlined" onClick={setDefaultViolin} sx={{ mr: 2 }}>Default Violin</Button>
        <Button variant="outlined" onClick={setDefaultViola} sx={{ mr: 2 }}>Default Viola</Button>
        <Button variant="contained" onClick={saveToFile}>Save Frequencies</Button>
    </Box>
  );
}

export default Tuner;
