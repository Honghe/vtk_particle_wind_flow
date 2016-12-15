//
// Created by honhe on 12/14/16.
//

#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPointData.h>
#include <vtkLookupTable.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include "WindActorWrapper.h"
#include "vtkPolyData.h"

WindActorWrapper::WindActorWrapper(vtkSmartPointer<vtkRenderWindow> renderWin,
                                   vtkSmartPointer<vtkRenderWindowInteractor> renderInteractor) {
    this->renderWin = renderWin;
    this->renderInteractor = renderInteractor;
}

void WindActorWrapper::init() {
    polyData = vtkSmartPointer<vtkPolyData>::New();
    points = vtkSmartPointer<vtkPoints>::New();
    vertices = vtkSmartPointer<vtkCellArray>::New();
    scalars = vtkSmartPointer<vtkUnsignedCharArray>::New();
    pointsBase = vtkSmartPointer<vtkPoints>::New();
}

void WindActorWrapper::createData() {

    points->SetDataTypeToFloat();
    points->Reset();

    scalars->SetName("Scalar");
    for (int i = 0; i < data_axis_x; ++i) {
        for (int j = 0; j < data_axis_y; ++j) {
            for (int k = 0; k < data_axis_z; ++k) {
                // 扩大点的倍数
                for (int l = 0; l < 4; ++l) {
                    // for simulate liquid/flow, the particle need add random movement for better
                    double x = i - rand() % 20 * 0.05;
                    // 主要初始给个随机，随后可以不用偏移，如何随后随机偏移的话看过去动态比较厉害
                    double y = j - (rand() % 20 - 10) * 0.05;
                    double z = k - (rand() % 20 - 10) * 0.05;
                    points->InsertNextPoint(x, y, z);
                    scalars->InsertNextValue(0);
                }
            }
        }
    }
    // copy 一份原始点数据，后面复位时用
    pointsBase->DeepCopy(points);

    // 所以点都放在同一个 Vert 中
    for (vtkIdType j = 0; j < (vtkIdType) points->GetNumberOfPoints(); ++j) {
        vertices->InsertNextCell(1);
        vertices->InsertCellPoint(j);
    }

    polyData->SetPoints(points);
    polyData->SetVerts(vertices);
    polyData->GetPointData()->SetScalars(scalars);
    polyData->Modified();

    vtkSmartPointer<vtkPolyDataMapper> mapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);
    mapper->SetScalarRange(0, 255);

    const vtkSmartPointer<vtkLookupTable> &lut = vtkSmartPointer<vtkLookupTable>::New();
    lut->SetNumberOfTableValues(colorNumber);
    lut->Build();
    for (int l = 0; l < colorNumber; ++l) {
        double *pDouble = lut->GetTableValue(l);
        // 随距离不一样，Alpha不渐变
        lut->SetTableValue(l, pDouble[0], pDouble[1], pDouble[2], float(l) / (colorNumber * 10) + 0.005);
    }

    mapper->SetLookupTable(lut);
    mapper->SetColorModeToMapScalars();
    mapper->SetScalarModeToUsePointData();

    const vtkSmartPointer<vtkProperty> actorProperty = vtkSmartPointer<vtkProperty>::New();
//    actorProperty->SetColor(colorTransferFunction);
//    actorProperty->SetScalarOpacity(alphaChannelFunction);
//    actorProperty->SetOpacity(0.3);

    actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->SetProperty(actorProperty);
    actor->GetProperty()->SetPointSize(4);
    mapper->Update();
}

void WindActorWrapper::refreshWind() {
    double point[3] = {0};
    for (int i = 0; i < data_axis_x; ++i) {
        for (int j = 0; j < data_axis_y; ++j) {
            for (int k = 0; k < data_axis_z; ++k) {
                int id = data_axis_y * data_axis_z * i + data_axis_z * j + k;
                id *= 4;
                // 扩大点的倍数
                for (int l = 0; l < 4; ++l) {
                    id += l;
                    points->GetPoint(id, point);

                    // 动动方向上加速
                    double xDelta = rand() % 20 * 0.05 * (((float) point[0] / data_axis_x));
                    double pointx = point[0] - xDelta;
                    // y 轴方向上弯曲，模拟抛物线
                    double pointz;
                    if (useParabola) {
                        pointz = point[2] + 0.5 * pow((1 - pointx / data_axis_x), 2);
                    } else {
                        pointz = point[2];
                    }
                    // 随机生命周期结束 或者 超出范围，就回到初始位置
                    if (rand() % 20 == 0 || (pointx > data_axis_x - 1 || pointx < 0.05) || pointz > 20) {
                        pointx = data_axis_x - 1;
                        // y轴方向恢复
                        if (useParabola) {
                            double tmpPoint[3];
                            pointsBase->GetPoint(id, tmpPoint);
                            pointz = tmpPoint[2];
                        }
                    }
                    // 更新点的状态与属性
                    points->SetPoint(id, pointx, point[1], pointz);
                    scalars->SetValue(id, pointx * 2);
                }
            }
        }
    }
    points->Modified();
    this->renderWin->Render();
}
