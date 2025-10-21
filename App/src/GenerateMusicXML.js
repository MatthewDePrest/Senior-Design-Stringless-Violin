function generateMusicXML(notes){
    let measures = notes.map((n, i) => `
        <measure number = "${i + 1}">
            <note>
                <pitch>
                    <step>${n.note[0]}</step>
                    ${n.note.includes('#') ? '<alter>1</alter>' : ''}
                    <octave>${n.octave}</octave>
                </pitch>
                <duration>${n.duration}</duration>
                <type>${n.type}</type>
            </note>
        </measure>`).join('');
    return `
    <score-partwise version="3.1">
        <part-list>
            <score-part id="P1">
                <part-name>Generated</part-name>
            </score-part>
        </part-list>
        <part id="P1">
            ${measures}
        </part>
    </score-partwise>`;
}

    export default generateMusicXML;