package org.lflang.generator;

import java.util.Properties;

import org.eclipse.xtext.util.CancelIndicator;

import org.lflang.ErrorReporter;
import org.lflang.FileConfig;
import org.lflang.TargetConfig.Mode;

/**
 * A {@code NestedContext} is the context of a process within a build process.
 */
public class NestedContext implements LFGeneratorContext {

    private final LFGeneratorContext containingContext;
    private final int startPercentProgress;
    private final int endPercentProgress;

    /**
     * Initializes the context within {@code containingContext} of the process that extends from
     * {@code startPercentProgress} to {@code endPercentProgress}.
     * @param containingContext The context of the containing build process.
     * @param startPercentProgress The percent progress of the containing build process when this
     *                             nested process starts.
     * @param endPercentProgress The percent progress of the containing build process when this
     *                           nested process ends.
     */
    public NestedContext(LFGeneratorContext containingContext, int startPercentProgress, int endPercentProgress) {
        this.containingContext = containingContext;
        this.startPercentProgress = startPercentProgress;
        this.endPercentProgress = endPercentProgress;
    }

    @Override
    public CancelIndicator getCancelIndicator() {
        return containingContext.getCancelIndicator();
    }

    @Override
    public Mode getMode() {
        return containingContext.getMode();
    }

    @Override
    public Properties getArgs() {
        return containingContext.getArgs();
    }

    @Override
    public boolean isHierarchicalBin() {
        return containingContext.isHierarchicalBin();
    }

    @Override
    public ErrorReporter constructErrorReporter(FileConfig fileConfig) {
        throw new UnsupportedOperationException(
            "Nested contexts should use the error reporters constructed by their containing contexts."
        );
    }

    @Override
    public void finish(GeneratorResult result) {
        // Do nothing. A build process is not finished until the outermost containing context is finished.
    }

    @Override
    public GeneratorResult getResult() {
        throw new UnsupportedOperationException("Only the outermost context can have a final result.");
    }

    @Override
    public void reportProgress(String message, int percentage) {
        containingContext.reportProgress(
            message,
            startPercentProgress * (100 - percentage) / 100 + endPercentProgress * percentage / 100
        );
    }
}
