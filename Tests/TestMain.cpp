#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() function
#include "catch.hpp"
#include "PluginEditor.h" 

TEST_CASE( "MIDI Notes are correctly quantized to scales", "[quantize]" ) {
    
    // We would instantiate your processor/editor here (or a standalone quantization class)
    // VirtualMidiKeyAudioProcessorEditor editor(...);

    SECTION( "Strict Quantize OFF allows all notes" ) {
        // Assuming strict quantize is off, feeding it a C# (61) should return C# (61)
        REQUIRE( editor.quantizeNote(61) == 61 );
    }

    SECTION( "C Major Scale Quantization" ) {
        // Set scale to C Major in your parameters here...
        
        // C (60) is in C Major, should remain C (60)
        REQUIRE( editor.quantizeNote(60) == 60 );
        
        // C# (61) is NOT in C Major. Your logic should push it to C (60) or D (62)
        // Let's assume your logic snaps down to C (60)
        REQUIRE( editor.quantizeNote(61) == 60 ); 
    }
}