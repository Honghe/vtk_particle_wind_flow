//
// Created by honhe on 12/14/16.
//

#include <vtkVersion.h>
#include <vtkSmartPointer.h>
#include <vtkSampleFunction.h>
#include <vtkSphereSource.h>
#include <vtkCamera.h>
#include <vtkCallbackCommand.h>
#include <vtkTransform.h>
#include <vtkProbeFilter.h>
#include <vtkPlanes.h>
#include <vtkDataSetMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkFrustumSource.h>
#include <vtkAssembly.h>
#include "WindActorWrapper.h"

using namespace std;

// Create a renderer, render window, and interactor
vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();
vtkSmartPointer<vtkRenderWindow> renderWindow =
        vtkSmartPointer<vtkRenderWindow>::New();
vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
        vtkSmartPointer<vtkRenderWindowInteractor>::New();

WindActorWrapper *windActorWrapper1;
WindActorWrapper *windActorWrapper2;
WindActorWrapper *windActorWrapper3;
WindActorWrapper *windActorWrapper4;


void CallbackFunction(vtkObject *caller, long unsigned int vtkNotUsed(eventId), void * vtkNotUsed(clientData), void *
                      vtkNotUsed(callData)) {
    vtkRenderer *renderer = static_cast<vtkRenderer *>(caller);

    double timeInSeconds = renderer->GetLastRenderTimeInSeconds();
    double fps = 1.0 / timeInSeconds;
    std::cout << "FPS: " << fps << std::endl;
}

class WindTimerCallback : public vtkCommand {
public:
    static WindTimerCallback *New() {
        return new WindTimerCallback;
    }

    virtual void Execute(vtkObject *vtkNotUsed(caller),
                         unsigned long event,
                         void *calldata) {
        windActorWrapper1->refreshWind();
        windActorWrapper2->refreshWind();
        windActorWrapper3->refreshWind();
        windActorWrapper4->refreshWind();
    }
};

vtkSmartPointer<vtkOrientationMarkerWidget> orientationMarkerWidget;

vtkSmartPointer<vtkActor> createAirConditionMachine() {
    vtkSmartPointer<vtkCamera> camera =
            vtkSmartPointer<vtkCamera>::New();
    double planesArray[24];

    camera->GetFrustumPlanes(1, planesArray);

    // 通过6个面放大截锥大小
    int scale = 40;
    for (int i = 0; i < 6; ++i) {
        planesArray[3 + i * 4] *= scale;
    }
    // 截断成合适的形状
    planesArray[18] = 1;
    planesArray[19] = 0;

    planesArray[22] = -10;
    planesArray[23] = scale;

    vtkSmartPointer<vtkPlanes> planes =
            vtkSmartPointer<vtkPlanes>::New();
    planes->SetFrustumPlanes(planesArray);

    vtkSmartPointer<vtkFrustumSource> frustumSource =
            vtkSmartPointer<vtkFrustumSource>::New();
    frustumSource->ShowLinesOn();
    frustumSource->SetPlanes(planes);
    frustumSource->Update();

    vtkPolyData *frustum = frustumSource->GetOutput();

    vtkSmartPointer<vtkPolyDataMapper> mapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(frustum);

    const vtkSmartPointer<vtkActor> &airConditionMachine = vtkSmartPointer<vtkActor>::New();
    airConditionMachine->SetMapper(mapper);
    return airConditionMachine;
}

/**
 * 用Point来做Particle粒子系统，从而模拟空调出风
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {
    int airConditionEdgeLength = 18;
    int windFlowDegree = 5;
    // Render and interact
    renderWindow->SetSize(600, 600);
    renderWindow->AddRenderer(renderer);
    renderWindowInteractor->SetRenderWindow(renderWindow);
    // do anything after Initialize, otherwise will throw error
    renderWindowInteractor->Initialize();

    //
    const vtkSmartPointer<vtkActor> &airConditionMachine = createAirConditionMachine();

    vtkCamera *pCamera = renderer->GetActiveCamera();
    pCamera->SetFocalPoint(airConditionMachine->GetCenter());
    pCamera->SetPosition(0, 0, -150);
    pCamera->Elevation(-80);
//    pCamera->Azimuth(50);
    //
    // #1
    windActorWrapper1 = new WindActorWrapper(renderWindow, renderWindowInteractor);
    windActorWrapper1->init();
    windActorWrapper1->createData();
    const vtkSmartPointer<vtkTransform> &transform1 = vtkSmartPointer<vtkTransform>::New();
    transform1->RotateY(windFlowDegree);
    transform1->Translate(-((windActorWrapper1->actor->GetXRange()[1] - windActorWrapper1->actor->GetXRange()[0]) +
                            airConditionEdgeLength / 2.0),
                          -windActorWrapper1->actor->GetYRange()[1] / 2.0,
                          0);
    windActorWrapper1->actor->SetUserTransform(transform1);

    // #2
    windActorWrapper2 = new WindActorWrapper(renderWindow, renderWindowInteractor);
    windActorWrapper2->init();
    windActorWrapper2->createData();
    const vtkSmartPointer<vtkTransform> &transform2 = vtkSmartPointer<vtkTransform>::New();
    transform2->PostMultiply();     // 右乘
    // 可以多个链接变换
    transform2->RotateZ(180);
    transform2->Translate((windActorWrapper2->actor->GetXRange()[1] - windActorWrapper2->actor->GetXRange()[0]) +
                          (airConditionEdgeLength) / 2.0, 0, 0);
    transform2->Translate(0, 10, 0);
    transform2->RotateY(-windFlowDegree);
    windActorWrapper2->actor->SetUserTransform(transform2);

    // #3
    windActorWrapper3 = new WindActorWrapper(renderWindow, renderWindowInteractor);
    windActorWrapper3->init();
    windActorWrapper3->createData();
    const vtkSmartPointer<vtkTransform> &transform3 = vtkSmartPointer<vtkTransform>::New();
    transform3->PostMultiply();     // 右乘
    // 可以多个链接变换
    transform3->RotateZ(90);
    // 注意，上面 Rotatez后，这里的x轴方向就要对应取GetYRange
    transform3->Translate((windActorWrapper3->actor->GetYRange()[1] - windActorWrapper3->actor->GetYRange()[0]) / 2.0,
                          -((windActorWrapper3->actor->GetXRange()[1] - windActorWrapper3->actor->GetXRange()[0]) +
                            airConditionEdgeLength / 2.0),
                          0);
    transform3->RotateX(-windFlowDegree);
    windActorWrapper3->actor->SetUserTransform(transform3);

    // #4
    windActorWrapper4 = new WindActorWrapper(renderWindow, renderWindowInteractor);
    windActorWrapper4->init();
    windActorWrapper4->createData();
    const vtkSmartPointer<vtkTransform> &transform4 = vtkSmartPointer<vtkTransform>::New();
    transform4->PostMultiply();     // 右乘
    // 可以多个链接变换
    transform4->RotateZ(-90);
    //
    transform4->Translate(-(windActorWrapper4->actor->GetYRange()[1] - windActorWrapper4->actor->GetYRange()[0]) / 2.0,
                          ((windActorWrapper4->actor->GetXRange()[1] - windActorWrapper4->actor->GetXRange()[0]) +
                           airConditionEdgeLength / 2.0),
                          0);
    transform4->RotateX(windFlowDegree);
    windActorWrapper4->actor->SetUserTransform(transform4);

    // Combine the sphere and cube into an assembly
    vtkSmartPointer<vtkAssembly> assembly =
            vtkSmartPointer<vtkAssembly>::New();
    assembly->AddPart(windActorWrapper1->actor);
    assembly->AddPart(windActorWrapper2->actor);
    assembly->AddPart(windActorWrapper3->actor);
    assembly->AddPart(windActorWrapper4->actor);
    assembly->AddPart(airConditionMachine);

    renderer->AddActor(assembly);

    const vtkSmartPointer<vtkAxesActor> &axesActor = vtkSmartPointer<vtkAxesActor>::New();
    orientationMarkerWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    orientationMarkerWidget->SetOrientationMarker(axesActor);
    orientationMarkerWidget->SetInteractor(renderWindowInteractor);
    orientationMarkerWidget->SetCurrentRenderer(renderer);
    orientationMarkerWidget->SetViewport(0, 0, 0.2, 0.2);;
    orientationMarkerWidget->SetEnabled(1);
    orientationMarkerWidget->SetInteractive(1);

    renderWindow->Render();

    vtkSmartPointer<WindTimerCallback> timerCallback = vtkSmartPointer<WindTimerCallback>::New();
    renderWindowInteractor->AddObserver(vtkCommand::TimerEvent, timerCallback);
    int interval = 30;
    renderWindowInteractor->CreateRepeatingTimer(interval);

//    // fps
    vtkSmartPointer<vtkCallbackCommand> callback =
            vtkSmartPointer<vtkCallbackCommand>::New();
    callback->SetCallback(CallbackFunction);
    renderer->AddObserver(vtkCommand::EndEvent, callback);

    renderWindowInteractor->Start();

    return EXIT_SUCCESS;
}

