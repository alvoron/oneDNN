/*******************************************************************************
* Copyright 2017-2021 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#ifndef CPU_X64_JIT_UNI_I8I8_POOLING_HPP
#define CPU_X64_JIT_UNI_I8I8_POOLING_HPP

#include "common/c_types_map.hpp"
#include "common/primitive.hpp"
#include "common/type_helpers.hpp"

#include "cpu/cpu_pooling_pd.hpp"

#include "cpu/x64/cpu_isa_traits.hpp"
#include "cpu/x64/jit_primitive_conf.hpp"

namespace dnnl {
namespace impl {
namespace cpu {
namespace x64 {

template <cpu_isa_t isa>
struct jit_uni_i8i8_pooling_fwd_ker_t;

template <cpu_isa_t isa>
struct jit_uni_i8i8_pooling_fwd_t : public primitive_t {
    struct pd_t : public cpu_pooling_fwd_pd_t {
        using cpu_pooling_fwd_pd_t::cpu_pooling_fwd_pd_t;

        DECLARE_COMMON_PD_T(JIT_IMPL_NAME_HELPER("jit_int:", isa, ""),
                jit_uni_i8i8_pooling_fwd_t);

        status_t init(engine_t *engine) {
            using namespace format_tag;
            bool ok = mayiuse(isa) && utils::one_of(ndims(), 3, 4, 5)
                    && desc()->prop_kind == prop_kind::forward_inference
                    && utils::one_of(desc()->alg_kind, alg_kind::pooling_max,
                            alg_kind::pooling_avg_include_padding,
                            alg_kind::pooling_avg_exclude_padding)
                    && utils::one_of(src_md()->data_type, data_type::s32,
                            data_type::s8, data_type::u8)
                    && !is_dilated()
                    && attr()->has_default_values(
                            primitive_attr_t::skip_mask_t::post_ops)
                    && IMPLICATION(utils::one_of(desc()->alg_kind, alg_kind::pooling_avg_include_padding, alg_kind::pooling_avg_exclude_padding),
                                   utils::one_of(dst_md()->data_type, data_type::u8, data_type::s8, data_type::f32))
                    && set_default_params() == status::success
                    && memory_desc_matches_one_of_tag(
                               *src_md(), nwc, nhwc, ndhwc)
                            != format_tag::undef
                    && memory_desc_matches_one_of_tag(
                               *dst_md(), nwc, nhwc, ndhwc)
                            != format_tag::undef
                    && attr_.set_default_formats(dst_md(0)) == status::success;
            if (!ok) return status::unimplemented;

            CHECK(jit_conf());

            return status::success;
        }

        jit_pool_conf_t jpp_;

    protected:
        status_t jit_conf();
    };

    jit_uni_i8i8_pooling_fwd_t(const pd_t *apd);
    ~jit_uni_i8i8_pooling_fwd_t();

    status_t init(engine_t *engine) override;

    status_t execute(const exec_ctx_t &ctx) const override {
        return execute_forward(ctx);
    }

private:
    status_t execute_forward(const exec_ctx_t &ctx) const;
    const pd_t *pd() const { return (const pd_t *)primitive_t::pd().get(); }

    std::unique_ptr<jit_uni_i8i8_pooling_fwd_ker_t<isa>> ker_;
};

} // namespace x64
} // namespace cpu
} // namespace impl
} // namespace dnnl

#endif
