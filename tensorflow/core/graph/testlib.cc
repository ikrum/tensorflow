/* Copyright 2015 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/core/graph/testlib.h"

#include <vector>
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/framework/node_def_builder.h"
#include "tensorflow/core/framework/types.h"
#include "tensorflow/core/framework/types.pb.h"
#include "tensorflow/core/graph/graph.h"
#include "tensorflow/core/graph/node_builder.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/public/status.h"

namespace tensorflow {
namespace test {
namespace graph {

Node* Send(Graph* g, Node* input, const string& tensor, const string& sender,
           const uint64 sender_incarnation, const string& receiver) {
  Node* ret;
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), "_Send")
                  .Input(input, 0)
                  .Attr("tensor_name", tensor)
                  .Attr("send_device", sender)
                  .Attr("send_device_incarnation",
                        static_cast<int64>(sender_incarnation))
                  .Attr("recv_device", receiver)
                  .Finalize(g, &ret));
  return ret;
}

Node* Recv(Graph* g, const string& tensor, const string& type,
           const string& sender, const uint64 sender_incarnation,
           const string& receiver) {
  Node* ret;
  DataType dtype;
  CHECK(DataTypeFromString(type, &dtype));
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), "_Recv")
                  .Attr("tensor_type", dtype)
                  .Attr("tensor_name", tensor)
                  .Attr("send_device", sender)
                  .Attr("send_device_incarnation",
                        static_cast<int64>(sender_incarnation))
                  .Attr("recv_device", receiver)
                  .Finalize(g, &ret));
  return ret;
}

Node* Constant(Graph* g, const Tensor& tensor) {
  Node* ret;
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), "Const")
                  .Attr("dtype", tensor.dtype())
                  .Attr("value", tensor)
                  .Finalize(g, &ret));
  return ret;
}

Node* Constant(Graph* g, const Tensor& tensor, const string& name) {
  Node* ret;
  TF_CHECK_OK(NodeBuilder(name, "Const")
                  .Attr("dtype", tensor.dtype())
                  .Attr("value", tensor)
                  .Finalize(g, &ret));
  return ret;
}

Node* Var(Graph* g, const DataType dtype, const TensorShape& shape) {
  Node* ret;
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), "Variable")
                  .Attr("dtype", dtype)
                  .Attr("shape", shape)
                  .Finalize(g, &ret));
  return ret;
}

Node* Assign(Graph* g, Node* var, Node* val) {
  Node* ret;
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), "Assign")
                  .Input(var)
                  .Input(val)
                  .Attr("use_locking", true)
                  .Finalize(g, &ret));
  return ret;
}

Node* Reduce(Graph* g, const string& reduce, Node* data, Node* axes,
             bool keep_dims) {
  Node* ret;
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), reduce)
                  .Input(data)
                  .Input(axes)
                  .Attr("keep_dims", keep_dims)
                  .Finalize(g, &ret));
  return ret;
}

Node* QuantizeToUINT8(Graph* g, Node* data) {
  Node* ret;
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), "Quantize")
                  .Input(data)
                  .Attr("T", DT_QUINT8)
                  .Attr("max_range", 1.0f)
                  .Attr("min_range", -1.0f)
                  .Finalize(g, &ret));
  return ret;
}

Node* Matmul(Graph* g, Node* in0, Node* in1, bool transpose_a,
             bool transpose_b) {
  Node* ret;
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), "MatMul")
                  .Input(in0)
                  .Input(in1)
                  .Attr("transpose_a", transpose_a)
                  .Attr("transpose_b", transpose_b)
                  .Finalize(g, &ret));
  return ret;
}

Node* RandomNumberGenerator(const string& op, Graph* g, Node* input,
                            DataType dtype) {
  Node* ret;
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), op)
                  .Input(input)
                  .Attr("dtype", dtype)
                  .Attr("seed", 0)
                  .Finalize(g, &ret));
  return ret;
}

Node* RandomUniform(Graph* g, Node* input, DataType dtype) {
  return RandomNumberGenerator("RandomUniform", g, input, dtype);
}

Node* RandomGaussian(Graph* g, Node* input, DataType dtype) {
  return RandomNumberGenerator("RandomStandardNormal", g, input, dtype);
}

Node* RandomParameters(Graph* g, Node* input, DataType dtype) {
  return RandomNumberGenerator("RandomParameters", g, input, dtype);
}

Node* Unary(Graph* g, const string& func, Node* input, int index) {
  Node* ret;
  TF_CHECK_OK(
      NodeBuilder(g->NewName("n"), func).Input(input, index).Finalize(g, &ret));
  return ret;
}

Node* Binary(Graph* g, const string& func, Node* in0, Node* in1) {
  Node* ret;
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), func)
                  .Input(in0)
                  .Input(in1)
                  .Finalize(g, &ret));
  return ret;
}

Node* Multi(Graph* g, const string& func, gtl::ArraySlice<Node*> ins) {
  Node* ret;
  auto b = NodeBuilder(g->NewName("n"), func);
  for (Node* n : ins) b = b.Input(n);
  TF_CHECK_OK(b.Finalize(g, &ret));
  return ret;
}

Node* Identity(Graph* g, Node* input, int index) {
  Node* ret;
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), "Identity")
                  .Input(input, index)
                  .Finalize(g, &ret));
  return ret;
}

Node* Add(Graph* g, Node* in0, Node* in1) { return Binary(g, "Add", in0, in1); }

Node* Error(Graph* g, Node* input, const string& errmsg) {
  Node* ret;
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), "Error")
                  .Input(input)
                  .Attr("message", errmsg)
                  .Finalize(g, &ret));
  return ret;
}

Node* InvalidRefType(Graph* g, DataType out_type, DataType invalid_type) {
  DCHECK(out_type != invalid_type);
  Node* ret;
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), "InvalidRefType")
                  .Attr("TIn", out_type)
                  .Attr("TOut", invalid_type)
                  .Finalize(g, &ret));
  return ret;
}

Node* Delay(Graph* g, Node* input, Microseconds delay_micros) {
  Node* ret;
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), "Delay")
                  .Input(input)
                  .Attr("micros", delay_micros.value())
                  .Finalize(g, &ret));
  return ret;
}

Node* NoOp(Graph* g, const std::vector<Node*>& control_inputs) {
  Node* ret;
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), "NoOp")
                  .ControlInputs(control_inputs)
                  .Finalize(g, &ret));
  return ret;
}

Node* Switch(Graph* g, Node* in0, Node* in1) {
  Node* ret;
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), "Switch")
                  .Input(in0)
                  .Input(in1)
                  .Finalize(g, &ret));
  return ret;
}

Node* Enter(Graph* g, Node* input, const string& frame_name) {
  Node* ret;
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), "Enter")
                  .Input(input)
                  .Attr("frame_name", frame_name)
                  .Finalize(g, &ret));
  return ret;
}

Node* Exit(Graph* g, Node* input) {
  Node* ret;
  TF_CHECK_OK(
      NodeBuilder(g->NewName("n"), "Exit").Input(input).Finalize(g, &ret));
  return ret;
}

Node* Merge(Graph* g, Node* in0, Node* in1) {
  Node* ret;
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), "Merge")
                  .Input({in0, in1})
                  .Finalize(g, &ret));
  return ret;
}

Node* Merge(Graph* g, Node* in0, gtl::ArraySlice<string> remaining_in) {
  std::vector<NodeBuilder::NodeOut> inputs;
  inputs.reserve(remaining_in.size() + 1);
  inputs.emplace_back(in0);
  for (const string& in_name : remaining_in) {
    inputs.emplace_back(in_name, 0, inputs[0].dt);
  }

  Node* ret;
  TF_CHECK_OK(
      NodeBuilder(g->NewName("n"), "Merge").Input(inputs).Finalize(g, &ret));
  return ret;
}

Node* Next(Graph* g, const string& name, Node* input) {
  Node* ret;
  TF_CHECK_OK(
      NodeBuilder(name, "NextIteration").Input(input).Finalize(g, &ret));
  return ret;
}

Node* LoopCond(Graph* g, Node* input) {
  Node* ret;
  TF_CHECK_OK(
      NodeBuilder(g->NewName("n"), "LoopCond").Input(input).Finalize(g, &ret));
  return ret;
}

Node* Less(Graph* g, Node* in0, Node* in1) {
  return Binary(g, "Less", in0, in1);
}

Node* Select(Graph* g, Node* c, Node* inx, Node* iny) {
  Node* ret;
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), "Select")
                  .Input(c)
                  .Input(inx)
                  .Input(iny)
                  .Finalize(g, &ret));
  return ret;
}

Node* Cast(Graph* g, Node* in, DataType dst) {
  Node* ret;
  TF_CHECK_OK(NodeBuilder(g->NewName("n"), "Cast")
                  .Input(in)
                  .Attr("DstT", dst)
                  .Finalize(g, &ret));
  return ret;
}

void ToGraphDef(Graph* g, GraphDef* gdef) { g->ToGraphDef(gdef); }

}  // end namespace graph
}  // end namespace test
}  // end namespace tensorflow
