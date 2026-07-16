package workflow

import (
	"context"
	"encoding/json"
	"fmt"
	"math"
	"math/rand"
	"os"
	"os/exec"
	"path/filepath"
	"time"
)

// AudioAnalysisResult holds features extracted from audio.
type AudioAnalysisResult struct {
	BPM         float64          `json:"bpm"`
	Key         string           `json:"key"`
	Scale       string           `json:"scale"`
	Confidence  float64          `json:"confidence"`
	Chords      []DetectedChord  `json:"chords"`
	Onsets      []float64        `json:"onsets"`
	DurationSec float64          `json:"duration_sec"`
}

// DetectedChord represents a single chord detected from audio.
type DetectedChord struct {
	Chord      string  `json:"chord"`
	TimeSec    float64 `json:"time_sec"`
	Confidence float64 `json:"confidence"`
}

// AudioAssistedRequest represents W3 input.
type AudioAssistedRequest struct {
	AudioPath  string // path to WAV file
	Seed       int64
	OutputPath string // optional; empty = temp file
	Genre      string // optional, for blueprint generation
}

// AudioAssistedResponse represents W3 output.
type AudioAssistedResponse struct {
	SMFPath    string
	Analysis   AudioAnalysisResult
	DurationMs int64
}

// AudioAssisted implements Workflow 3 (Audio Assisted Composer).
//
// It takes an audio file, runs DSP analysis (via dsp_cli), builds a
// MusicBlueprint from the extracted features, and generates an SMF via mte_cli.
func (wm *WorkflowManager) AudioAssisted(ctx context.Context, req *AudioAssistedRequest) (*AudioAssistedResponse, error) {
	if req.AudioPath == "" {
		return nil, fmt.Errorf("audio path must not be empty")
	}

	if _, err := os.Stat(req.AudioPath); err != nil {
		return nil, fmt.Errorf("audio file not accessible: %w", err)
	}

	wm.logger.InfoContext(ctx, "W3.AudioAssisted",
		"audio", req.AudioPath,
		"seed", req.Seed,
		"genre", req.Genre,
	)

	start := time.Now()

	// 1. Run audio analysis via dsp_cli
	analysisPath, err := wm.runDSP(ctx, req.AudioPath)
	if err != nil {
		return nil, fmt.Errorf("audio analysis failed: %w", err)
	}
	defer os.Remove(analysisPath)

	// 2. Parse JSON analysis
	analysisData, err := os.ReadFile(analysisPath)
	if err != nil {
		return nil, fmt.Errorf("read analysis result: %w", err)
	}

	var analysis AudioAnalysisResult
	if err := json.Unmarshal(analysisData, &analysis); err != nil {
		return nil, fmt.Errorf("parse analysis JSON: %w", err)
	}

	if analysis.Key == "" || analysis.Scale == "" {
		return nil, fmt.Errorf("analysis incomplete: key=%q scale=%q", analysis.Key, analysis.Scale)
	}

	// 3. Build blueprint from analysis and call mte_cli
	// Estimate number of bars from duration and BPM
	bpm := analysis.BPM
	if bpm < 1 {
		bpm = 120
	}
	bars := int(math.Round(analysis.DurationSec * bpm / 60.0 / 4.0))
	if bars < 1 {
		bars = 4
	}

	args := []string{
		"--key", analysis.Key,
		"--scale", analysis.Scale,
		"--bpm", fmt.Sprintf("%.0f", bpm),
		"--bars", fmt.Sprintf("%d", bars),
	}

	smfPath, err := wm.runMTE(ctx, args, req.Seed)
	if err != nil {
		return nil, fmt.Errorf("MTE generation failed: %w", err)
	}

	elapsed := time.Since(start)

	// 4. Optionally copy to user-specified path
	if req.OutputPath != "" {
		data, err := os.ReadFile(smfPath)
		if err != nil {
			return nil, fmt.Errorf("read generated SMF: %w", err)
		}
		if err := os.WriteFile(req.OutputPath, data, 0644); err != nil {
			return nil, fmt.Errorf("copy SMF to output path: %w", err)
		}
		os.Remove(smfPath)
		smfPath = req.OutputPath
	}

	wm.logger.InfoContext(ctx, "W3 audio assisted composition complete",
		"smf", smfPath,
		"key", analysis.Key,
		"scale", analysis.Scale,
		"bpm", analysis.BPM,
		"duration_ms", elapsed.Milliseconds(),
	)

	return &AudioAssistedResponse{
		SMFPath:    smfPath,
		Analysis:   analysis,
		DurationMs: elapsed.Milliseconds(),
	}, nil
}

// runDSP executes dsp_cli with the given audio path and returns the path
// to the analysis JSON file.
func (wm *WorkflowManager) runDSP(ctx context.Context, audioPath string) (string, error) {
	outputFile := filepath.Join(wm.tmpDir, fmt.Sprintf("analysis_%x.json", rand.Uint64()))

	args := []string{"--input", audioPath, "--output", outputFile}

	cmd := exec.CommandContext(ctx, wm.dspCliPath, args...)
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("dsp_cli failed: %w\noutput: %s", err, string(output))
	}

	if _, statErr := os.Stat(outputFile); statErr != nil {
		return "", fmt.Errorf("dsp_cli exited ok but output file %q not found: %w", outputFile, statErr)
	}

	return outputFile, nil
}
