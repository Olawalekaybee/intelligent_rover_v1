## Summary

Describe what this PR changes and why.

## Firmware node(s) affected

- [ ] main-controller
- [ ] ai-camera
- [ ] cyd-display
- [ ] docs / README only

## Type of change

- [ ] Bug fix
- [ ] New feature
- [ ] Performance improvement
- [ ] Refactor
- [ ] Documentation

## Testing

Describe how you tested this. Include serial monitor output for firmware changes.

```
paste relevant serial output here
```

## Checklist

- [ ] `pio run` succeeds on all affected nodes with no new warnings
- [ ] `Secrets.h` is not included in this PR
- [ ] New feature flags default to `0` in `AppConfig.h`
- [ ] I2C work is performed outside the telemetry mutex
- [ ] Stack sizes are sufficient (no `mprec.c` crash in sensor task)
- [ ] CHANGELOG.md updated