function frequencyToNote(freq) {
    const A4 = 440;
    const semitonesFromA4 = 12 * Math.log2(freq / A4);
    const midi = Math.round(69 + semitonesFromA4);
    const noteNames = ['C', 'C#', 'D', 'D#', 'E', 'F','F#', 'G', 'G#', 'A', 'A#', 'B'];
    const note = noteNames[midi % 12];
    const octave = Math.floor(midi / 12) -1;
    return { note, octave };
}
export default frequencyToNote;