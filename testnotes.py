import time
import random

notes = ['C', 'D', 'E', 'F', 'G', 'A', 'B']
octaves = [3, 4, 5]

file_name = "App/live/notes.txt"

def write_music_notes():
    with open(file_name, 'a') as file:
        while True:
            note = random.choice(notes)
            octave = random.choice(octaves)
            note_with_octave = f"{note}{octave}"
            file.write(note_with_octave + '\n')
            file.flush()
            print(f"Written note: {note_with_octave}")

            time.sleep(0.5)

if __name__ == "__main__":
    print(f"Writing music notes to {file_name} every 0.5 seconds...")
    write_music_notes()
