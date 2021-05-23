// Copyright 2019 Alpha Cephei Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef VOSK_KALDI_RECOGNIZER_H
#define VOSK_KALDI_RECOGNIZER_H

#include "base/kaldi-common.h"
#include "decoder/lattice-faster-decoder.h"
#include "feat/feature-mfcc.h"
#include "fstext/fstext-lib.h"
#include "fstext/fstext-utils.h"
#include "lat/compose-lattice-pruned.h"
#include "lat/kaldi-lattice.h"
#include "lat/word-align-lattice.h"
#include "model.h"
#include "nnet3/am-nnet-simple.h"
#include "nnet3/nnet-am-decodable-simple.h"
#include "nnet3/nnet-utils.h"
#include "spk_model.h"
#include "util/common-utils.h"

using namespace kaldi;

enum KaldiRecognizerState {
    RECOGNIZER_INITIALIZED,
    RECOGNIZER_RUNNING,
    RECOGNIZER_ENDPOINT,
    RECOGNIZER_FINALIZED
};

class KaldiRecognizer {
   public:
    KaldiRecognizer(Model *model, float sample_frequency);
    KaldiRecognizer(Model *model, SpkModel *spk_model, float sample_frequency);
    KaldiRecognizer(Model *model, float sample_frequency, char const *grammar);
    ~KaldiRecognizer();
    void SetMaxAlternatives(int max_alternatives);
    bool AcceptWaveform(const char *data, int len);
    bool AcceptWaveform(const short *sdata, int len);
    bool AcceptWaveform(const float *fdata, int len);
    void SetEndpointMustContainNonsilence(int rule_id, bool must_contain_nonsilence);
    void SetEndpointMinTrailingSilence(int rule_id, BaseFloat min_trailing_silence);
    void SetEndpointMaxRelativeCost(int rule_id, BaseFloat max_relative_cost);
    void SetEndpointMinUtteranceLength(int rule_id, BaseFloat min_utterance_length);
    const char *Result();
    const char *FinalResult();
    const char *PartialResult();

   private:
    void InitState();
    void InitRescoring();
    void CleanUp();
    void UpdateSilenceWeights();
    bool AcceptWaveform(Vector<BaseFloat> &wdata);
    OnlineEndpointRule *GetOnlineEndpointRule(int id);
    bool GetSpkVector(Vector<BaseFloat> &out_xvector, int *frames);
    const char *GetResult();
    const char *StoreEmptyReturn();
    const char *StoreReturn(const string &res);
    const char *MbrResult(CompactLattice &clat);
    const char *NbestResult(CompactLattice &clat);

    Model *model_ = nullptr;
    SingleUtteranceNnet3Decoder *decoder_ = nullptr;
    fst::LookaheadFst<fst::StdArc, int32> *decode_fst_ = nullptr;
    fst::StdVectorFst *g_fst_ = nullptr;  // dynamically constructed grammar
    OnlineNnet2FeaturePipeline *feature_pipeline_ = nullptr;
    OnlineSilenceWeighting *silence_weighting_ = nullptr;

    // Speaker identification
    SpkModel *spk_model_ = nullptr;
    OnlineBaseFeature *spk_feature_ = nullptr;

    // Rescoring
    fst::ArcMapFst<fst::StdArc, kaldi::LatticeArc, fst::StdToLatticeMapper<kaldi::BaseFloat>> *lm_fst_ = nullptr;

    // RNNLM rescoring
    kaldi::rnnlm::RnnlmComputeStateInfo *info = nullptr;
    fst::ScaleDeterministicOnDemandFst *lm_to_subtract_det_scale = nullptr;
    fst::BackoffDeterministicOnDemandFst<fst::StdArc> *lm_to_subtract_det_backoff = nullptr;
    kaldi::rnnlm::KaldiRnnlmDeterministicFst *lm_to_add_orig = nullptr;
    fst::DeterministicOnDemandFst<fst::StdArc> *lm_to_add = nullptr;

    int max_alternatives_ = 0;  // Disable alternatives by default

    float sample_frequency_;
    int32 frame_offset_;

    int64 samples_processed_;
    int64 samples_round_start_;

    KaldiRecognizerState state_;
    string last_result_;
};

#endif /* VOSK_KALDI_RECOGNIZER_H */
