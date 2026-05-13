/*
 * File:        analysis_tooling_contract_test.cpp
 * Module:      nn-ntsc-chroma-sink tests
 * Purpose:     Verify the public analysis-tool SDK contracts in a third-party workspace
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 decode-orc contributors
 */

#include <gtest/gtest.h>

#include <orc/plugin/orc_stage_tooling.h>

#include <string>
#include <vector>

namespace orc_unit_test {
namespace {

class DummyAnalysisToolProvider final : public orc::AnalysisToolProvider,
                                        public orc::StageToolProvider {
public:
    std::vector<orc::AnalysisToolDescriptor> get_analysis_tools() const override
    {
        return {
            {
                "nn-ntsc-analysis-tool",
                "NN NTSC Analysis",
                "Third-party analysis helper contract proof",
                "decode-orc.stage-tools.nn-ntsc-analysis.v1",
                true,
            }
        };
    }

    std::vector<orc::StageToolDescriptor> get_stage_tools() const override
    {
        return {
            {
                "nn-ntsc-preview-tool",
                "NN NTSC Preview",
                "Third-party preview helper contract proof",
                orc::StageToolKind::PreviewUtility,
                false,
                "decode-orc.stage-tools.nn-ntsc-preview.v1",
            }
        };
    }
};

} // namespace

TEST(AnalysisToolingContractTest, thirdPartyWorkspaceCanExpressAnalysisAndPreviewToolContracts)
{
    DummyAnalysisToolProvider provider;

    const auto analysis_tools = provider.get_analysis_tools();
    ASSERT_EQ(analysis_tools.size(), 1U);
    EXPECT_EQ(analysis_tools.front().tool_id, "nn-ntsc-analysis-tool");
    EXPECT_EQ(analysis_tools.front().contract_id, "decode-orc.stage-tools.nn-ntsc-analysis.v1");
    EXPECT_TRUE(analysis_tools.front().auto_request_after_trigger);

    const auto stage_tools = provider.get_stage_tools();
    ASSERT_EQ(stage_tools.size(), 1U);
    EXPECT_EQ(stage_tools.front().tool_id, "nn-ntsc-preview-tool");
    EXPECT_EQ(stage_tools.front().kind, orc::StageToolKind::PreviewUtility);
    EXPECT_FALSE(stage_tools.front().non_modal);
    EXPECT_EQ(stage_tools.front().contract_id, "decode-orc.stage-tools.nn-ntsc-preview.v1");
}

} // namespace orc_unit_test